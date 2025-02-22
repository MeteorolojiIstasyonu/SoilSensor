#include "ThingSpeakManager.h"
#include "Sim808Manager.h"
#include "LogManager.h"
#include "Config.h"

void ThingSpeakManager::sendDataToThingSpeak(int sensorValues[7]) {
    if (Sim808Manager::sendATCommand("AT+HTTPINIT", 2000) != 0) {
        LogManager::log(LOG_LVL_ERROR, "HTTP oturumu başlatilamadi.");
        return;
    }

    char url[256];
    snprintf(url, sizeof(url), "http://api.thingspeak.com/update?api_key=%s", THINGSPEAK_KEY);
    for (int i = 0; i < 7; ++i) {
        char field[20];
        snprintf(field, sizeof(field), "&field%d=%d", i + 1, sensorValues[i]);
        strcat(url, field);
    }

    char cmd[300];
    snprintf(cmd, sizeof(cmd), "AT+HTTPPARA=\"URL\",\"%s\"", url);
    if (Sim808Manager::sendATCommand(cmd, 2000) != 0) {
        LogManager::log(LOG_LVL_ERROR, "HTTP parametreleri ayarlanamadi.");
        Sim808Manager::sendATCommand("AT+HTTPTERM", 1000);
        return;
    }

    if (Sim808Manager::sendATCommand("AT+HTTPACTION=0", 5000) != 0) {
        LogManager::log(LOG_LVL_ERROR, "HTTP isteği başarisiz oldu.");
        Sim808Manager::sendATCommand("AT+HTTPTERM", 1000);
        return;
    }

    if (Sim808Manager::sendATCommand("AT+HTTPREAD", 5000) != 0) {
        LogManager::log(LOG_LVL_ERROR, "HTTP yaniti okunamadi.");
    } 
    else {
        LogManager::log(LOG_LVL_INFO, "Veriler Thingspeak'e başariyla gönderildi.");
    }

    if (Sim808Manager::sendATCommand("AT+HTTPTERM", 1000) != 0) {
        LogManager::log(LOG_LVL_WARN, "HTTP oturumu sonlandirilamadi.");
    }
}
