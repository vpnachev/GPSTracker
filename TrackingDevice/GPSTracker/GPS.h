#ifndef __GPS_H_
#define __GPS_H_


#include <LGPRS.h>
#include <LGPRSClient.h>
#include <LGPRSServer.h>
#include <LGPRSUdp.h>
#include <LGPS.h>

#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LWiFiServer.h>
#include <LWiFiUdp.h>

#define UPPER_TIME_LIMIT 120000     // The maximum sleep time in ms
#define LOWER_TIME_LIMIT 2000       // The minimum sleep time in ms
#define SITE_URL_LENGTH 256         // Maximum lenght of URL
#define CACHE_COUNT 8               // This number of last positions
#define MOVEMENT_TRESHHOLD 0.0005f 	// 0.0009322146 ~ 100m distance somewhere in Bulgaria. On poles it should be less.

template<typename T>
T max(const T& lhs, const T& rhs);

template<typename T>
T min(const T& lhs, const T& rhs);
char* parse_token(char* data, char* dest);

class GPSTracker
{
	LGPRSClient client;
	int time_interval;
	char site_url[SITE_URL_LENGTH];
	int site_port;
	unsigned long lastRun;
	unsigned cache_gps;
	gpsSentenceInfoStruct gps_info;
	bool Wifi_enabled;
	struct {
		double longitude;
		double latitude;
		double altitude;
		float date_time;
		int num_satelites;
	} gps_data[CACHE_COUNT];

	void localize();
	void submitToServer();
	void updateTimeInterval();
	void setUrl(const char * new_url);

	void startGps();
	void stopGps();
	void startGPRS();
	bool haveWeMoved() const;
	void serve_on_wifi() const;

public:
	GPSTracker();
	void initModules();
	void disableModules();
	void run();
};

#endif

