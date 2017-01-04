#include "Battery.h"

void report_battery_status()
{
    boolean isCharging = LBattery.isCharging();
    int batteryLevel = LBattery.level();
    char buff[256];
    sprintf(buff, "Battery level = %d%%; and %scharging",
    batteryLevel, isCharging ? "" : "dis");
}

int get_battery_rate()
{
    Serial.print("Battery rate is ");
    Serial.println(LBattery.level());
    return LBattery.level();
}
