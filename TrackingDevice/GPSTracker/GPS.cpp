#include "GPS.h"
#include "Battery.h"

char* parse_token(char* data, char* dest)
{
    int i = 0;
    while(data[i] != ',' && data[i] != 0)
        ++i;
    if (dest != NULL)
    {
        strncpy(dest, data, i);
        dest[i] = '\0';
    }
    if(data[i])
        ++i;
    return data + i;
}

void GPSTracker::localize()
{
    //Read data from GPS
    LGPS.getData(&gps_info);
    char* data = (char*)gps_info.GPGGA;
    Serial.println(data);
    char buff[20];
    data = parse_token(data, NULL); // $GPGGA
    data = parse_token(data, buff); // Time
    gps_data[cache_gps].date_time = atof(buff);

    data = parse_token(data, buff);   // Latitude
    double tmp = atof(buff);
    gps_data[cache_gps].latitude = ((int)tmp) / 100; 
    tmp -= (gps_data[cache_gps].latitude * 100);
    tmp /= 60.0;
    gps_data[cache_gps].latitude += tmp;

    data = parse_token(data, buff);   // North/South
    if(buff[0] == 'S' || buff[0] == 's')
        gps_data[cache_gps].latitude *= -1;

    data = parse_token(data, buff);   // Longitude
    tmp = atof(buff);
    gps_data[cache_gps].longitude = ((int)tmp) / 100; 
    tmp -= (gps_data[cache_gps].longitude * 100);
    tmp /= 60.0;
    gps_data[cache_gps].longitude += tmp; 

    data = parse_token(data, buff);   // West/East
    if(buff[0] == 'W' || buff[0] == 'w')
        gps_data[cache_gps].longitude *= -1;

    data = parse_token(data, buff);     // Fix bit
    if(buff[0] != '0')
    {
        data = parse_token(data, buff);
        gps_data[cache_gps].num_satelites = atoi(buff);
    }
    else
    {
        data = parse_token(data, NULL); // flush the empty number of satelites
        gps_data[cache_gps].num_satelites = 0;
    }
    data = parse_token(data, NULL); // flush the horizontal dilutoion of position
    data = parse_token(data, buff); // get the Altitude
    gps_data[cache_gps].altitude = atof(buff);
    data = parse_token(data, NULL); // flush the measurment system of altitude
    data = parse_token(data, buff); // get the height of geoid
    gps_data[cache_gps].altitude -= atof(buff); 

    Serial.println("The location was localized");
    Serial.print(gps_data[cache_gps].latitude, 6);
    Serial.print(" ");
    Serial.print(gps_data[cache_gps].longitude, 6);
    Serial.print(" #sats ");
    Serial.println(gps_data[cache_gps].num_satelites);
    delay(1000);
}

void GPSTracker::submitToServer()
{
    Serial.println("In submitToServer");
    int i;
    for (i = 0; i < 3; ++i)
    {
        Serial.print("connect to server, attempt <-- ");
        Serial.println(i + 1);
        if (client.connect(site_url, site_port))
        {
            break;
        }
    }
    if (i == 3)
    {
        Serial.println("Failed to connecto to site_url");
        return;
    }
    char buff[256];
    client.println("GET / HTTP/1.1");
    client.print("Host: ");
    client.print(site_url);
    client.print(":");
    client.println(site_port);
    client.print("Latitude: ");
    sprintf(buff, "%.6f", gps_data[cache_gps].latitude);
    client.println(buff);
    client.print("Longitude: ");
    sprintf(buff, "%.6f", gps_data[cache_gps].longitude);
    client.println(buff);
    client.print("Altitude: ");
    client.println(gps_data[cache_gps].altitude);
    client.print("Satelites_Count: ");
    client.println(gps_data[cache_gps].num_satelites);
    client.print("Localization_Time: ");
    client.println(gps_data[cache_gps].date_time);
    client.print("Battery_Rate: ");
    client.println(get_battery_rate());
    client.println();

    Serial.println("The data is sent to server, now gets the response");
    // get the response
    int response;
    while (client.available())
    {
        response = client.read(); // TODO: Read the response on larger batches, not byte by byte
        if (response < 0)
            break;
    }
    Serial.println("Response arrived");
}

void GPSTracker::updateTimeInterval()
{
    if (haveWeMoved())
    {
        if (time_interval > 32000)
            time_interval /= 4;
        else if (time_interval > LOWER_TIME_LIMIT)
            time_interval /= 2;
        time_interval = max(time_interval, LOWER_TIME_LIMIT);
    }
    else
    {
        if (time_interval < 32000)
            time_interval *= 4;
        else if (time_interval < UPPER_TIME_LIMIT)
            time_interval *= 2;
        time_interval = min(time_interval, UPPER_TIME_LIMIT);
    }
}

void GPSTracker::setUrl(const char* new_url)
{
    strncpy(site_url, new_url, SITE_URL_LENGTH - 1);
}

GPSTracker::GPSTracker()
{
    client = LGPRSClient();
    time_interval = (UPPER_TIME_LIMIT + LOWER_TIME_LIMIT) / 16;
    setUrl("", SET_URL_COMPILE_ERROR); // TODO: Enter valid URL/IP
    site_port = 80;
    lastRun = millis();
    Wifi_enabled = false;
    GPS_enabled = false;
    cache_gps = 0;
    Serial.println("GPS_Tracker is init");
}

void GPSTracker::initModules()
{
    Serial.println("In initModules");
    startGps();
    startGPRS();
    for(int i = 0; i < CACHE_COUNT; ++i)
        gps_data[i].longitude = gps_data[i].latitude = 0.0;
}

void GPSTracker::startGps()
{
    LGPS.powerOn();
    delay(1000);
    GPS_enabled = true;
    Serial.println("GPS is started");
}

void GPSTracker::stopGps()
{
    LGPS.powerOff();
    GPS_enabled = false;
    Serial.println("GPS is stopped");
}

void GPSTracker::startGPRS()
{
    while (!LGPRS.attachGPRS("telenorbg", "telenor", "")) // TODO: Enter valid values for your telecom operator.
    {
        delay(2000);
    }
    Serial.println("GSM/GPRS is started");
}

void GPSTracker::disableModules()
{
    stopGps();
}

void GPSTracker::run()
{
    unsigned long current_lapsed_time;
    int tries = 0;
    current_lapsed_time = millis();
    if (current_lapsed_time - lastRun > time_interval)
    {
        Serial.println("Start another instance of the MaiN LOOP");
        // here we have to get the location and submit it
        // also update lastRun, check the movement in the last positions
        // the sum of the coordinates devided by the number of points
        // give us the center of gravity
        // if we were stationary, then the distance between all points will be very small
        // if we start to move, then tha last point will move the center of gravity
        // enough, that it will get away from most of the points
        if(GPS_enabled == false)
            startGps();
        tries = 0;
        do
        {
            Serial.println("In do-while loop");
            localize();
            ++tries;
        }while(gps_data[cache_gps].num_satelites < 4 && tries < 16);

        Serial.println("Out of do-while loop");
        Serial.print("attempts = ");
        Serial.println(tries);

        if(tries < 16 )
        {
            submitToServer();
            updateTimeInterval();
            if (Wifi_enabled)
            {
                // submit localization to WI-FI module
                this->serve_on_wifi();
            }
            // increase cache_gps
            cache_gps = (cache_gps+1) % CACHE_COUNT;
        }
        lastRun = millis();
        if (time_interval > 32000)
            stopGps();
        delay(time_interval);
    }
}

bool GPSTracker::haveWeMoved() const
{
    double longi=0.0f, lati=0.0f;
    for (int i = 0; i < CACHE_COUNT; ++i)
    {
        longi += gps_data[i].longitude;
        lati += gps_data[i].latitude;
    }
    longi /= CACHE_COUNT;
    lati /= CACHE_COUNT;
    if( abs(longi - gps_data[cache_gps].longitude) + 
        abs(lati - gps_data[cache_gps].latitude) < MOVEMENT_TRESHHOLD)
        return false;
    return true;
}

void GPSTracker::serve_on_wifi() const
{
    LWiFi.begin();
    LWiFi.connect("GPS_Tracker");
    LWiFiClient client;
    client.connect("server", 80);
    
    char buff[256];
    sprintf(buff, "%.6f", gps_data[cache_gps].latitude);
    client.print(buff);
    client.print(" ");
    sprintf(buff, "%.6f", gps_data[cache_gps].longitude);
    client.print(buff);
    client.println();

    LWiFi.end();
}
