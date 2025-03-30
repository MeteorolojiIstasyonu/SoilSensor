#include "ModuleSerialGps.h"

ModuleSerialGps::ModuleSerialGps(ModuleSerialCore *core) : core(core) {}

int ModuleSerialGps::enable()
{
    LOG_INFO("GPS etkinleştiriliyor...");

    if (!core->writeCommand("AT+CGPSPWR=1", "OK", 2000) ||
        !core->writeCommand("AT+CGPSRST=0", "OK", 2000)) 
    {
        LOG_ERROR("GPS başlatma başarısız! Hata kodu: %d", ERR_GPS_INIT_FAIL);
        return GPS_FAIL;
    }

    LOG_INFO("GPS başarıyla etkinleştirildi.");
    return GPS_ENABLED;
}

void ModuleSerialGps::disable()
{
    LOG_INFO("GPS devre dışı bırakılıyor...");
    core->writeCommand("AT+CGPSPWR=0", "OK", 2000);
}

ModuleSerialGps::GpsData ModuleSerialGps::currentGpsData()
{
    LOG_DEBUG("GPS verisi alınıyor...");
    
    char response[200] = "";
    core->writeCommand("AT+CGPSINF=0", response, 200, 2000);

    GpsData gpsData = {0};

    if (strstr(response, "+CGPSINF:") != nullptr)
    {
        parseGpsData(&gpsData, response);
        LOG_INFO("GPS verisi alındı: Lat: %f, Lng: %f", gpsData.lat, gpsData.lng);
    }
    else
    {
        LOG_WARN("GPS verisi alınamadı! Hata kodu: %d", ERR_GPS_NO_SIGNAL);
    }

    return gpsData;
}

void ModuleSerialGps::parseGpsData(GpsData *gpsData, char *data)
{
    char *pch = strtok(data, " ,");
    pch = strtok(nullptr, " ,"); gpsData->mode = atoi(pch);
    pch = strtok(nullptr, " ,"); gpsData->lat = atof(pch);
    pch = strtok(nullptr, " ,"); gpsData->lng = atof(pch);
    pch = strtok(nullptr, " ,"); gpsData->alt = atof(pch);
    pch = strtok(nullptr, " ,"); strcpy(gpsData->UTCTime, pch);
    pch = strtok(nullptr, " ,"); strcpy(gpsData->TTFF, pch);
    pch = strtok(nullptr, " ,"); gpsData->sat = atoi(pch);
    pch = strtok(nullptr, " ,"); gpsData->speed = atof(pch);
    pch = strtok(nullptr, " ,"); gpsData->course = atof(pch);

    LOG_DEBUG("GPS ayrıştırıldı -> Mod: %d, Sat: %d, Hız: %f", gpsData->mode, gpsData->sat, gpsData->speed);
}
