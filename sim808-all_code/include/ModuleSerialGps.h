#ifndef _MODULE_SERIAL_GPS_H
#define _MODULE_SERIAL_GPS_H

#include "ModuleSerialCore.h"
#include "LoggerModule.h"
#include "ErrorCodes.h"

#define GPS_FAIL -1
#define GPS_ENABLED 0

class ModuleSerialGps
{
public:
    struct GpsData
    {
        int mode;
        float lng, lat;
        float alt;
        char UTCTime[24];
        char TTFF[24];
        int sat;
        float speed;
        float course;
    };

    ModuleSerialGps(ModuleSerialCore *core);

    int enable();
    void disable();
    GpsData currentGpsData();

private:
    ModuleSerialCore *core;

    void parseGpsData(GpsData *gpsData, char *data);
};

#endif // _MODULE_SERIAL_GPS_H
