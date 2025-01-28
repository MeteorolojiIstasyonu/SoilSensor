// main.cpp
#include <Arduino.h>
#include "Config.h"
#include "LogManager.h"
#include "Sim808Manager.h"
#include "ThingSpeakManager.h"

// Sensör örnek değerleri
int sensorValues[7] = {0};

// Zaman yönetimi için değişken
unsigned long previousSendTime = 0;

void setup() {
    // Seri port (debug amaçlı)
    Serial.begin(115200);
    delay(1000);

    LogManager::log(LOG_LVL_INFO, "Sistem baslatiliyor...");

    // SIM808 seri başlatma
    Sim808Manager::initializeSIM808();

    // GPRS bağlantısı kontrolü
    Sim808Manager::checkGPRSConnection();
}
  

void loop() {
    // Periyodik olarak ThingSpeak'e veri gönder
    unsigned long currentTime = millis();
    if (currentTime - previousSendTime >= SEND_INTERVAL) {
        previousSendTime = currentTime;

        for (int i = 0; i < 7; ++i) {
            sensorValues[i] = random(10, 100);
        }

        ThingSpeakManager::sendDataToThingSpeak(sensorValues);
    }
    
    // Yield control for other tasks
    yield();
}
