#include "Sim808Manager.h"
#include "LogManager.h"
#include "Config.h"

HardwareSerial sim808(2);

void Sim808Manager::initializeSIM808() {
    sim808.begin(9600, SERIAL_8N1, SIM808_RX_PIN, SIM808_TX_PIN);
    LogManager::log(LOG_LVL_INFO, "SIM808 seri 9600 başlatildi");

    for (int attempt = 1; attempt <= 3; ++attempt) {
        if (sendATCommand("ATE0", 1000) == 0) {
            LogManager::log(LOG_LVL_INFO, "SIM808 başariyla başlatildi.");
            return;
        }
        LogManager::log(LOG_LVL_WARN, "SIM808 başlatma başarisiz oldu. Deniyorum...");
    }

    LogManager::log(LOG_LVL_ERROR, "3 Denemeden sonra SIM808 başlatma başarisiz oldu.");
}

bool Sim808Manager::connectToGPRS() {
    for (int attempt = 1; attempt <= 3; ++attempt) {
        sendATCommand("AT+SAPBR=0,1", 3000);  // Close previous session
        sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", 1000);

        char cmdBuffer[64];
        snprintf(cmdBuffer, sizeof(cmdBuffer), "AT+SAPBR=3,1,\"APN\",\"%s\"", APN_NAME);
        if (sendATCommand(cmdBuffer, 3000) != 0) {
            LogManager::log(LOG_LVL_WARN, "APN ayarlanamadi. Deniyorum...");
            continue;
        }

        if (sendATCommand("AT+SAPBR=1,1", 10000) == 0) {
            LogManager::log(LOG_LVL_INFO, "GPRS bağlantisi kuruldu.");
            return true;
        }

        LogManager::log(LOG_LVL_WARN, "GPRS bağlanti girişimi başarisiz oldu. Deniyorum...");
    }

    LogManager::log(LOG_LVL_ERROR, "3 Denemeden sonra GPRS bağlantisi kurulamadi.");
    return false;
}

void Sim808Manager::checkGPRSConnection() {
    if (!connectToGPRS()) {
        LogManager::log(LOG_LVL_ERROR, "GPRS bağlantisi kurulamiyor..");
    }
}

int Sim808Manager::sendATCommand(const char* cmd, unsigned long timeoutMs) {
    while (sim808.available()) sim808.read();  // Clear serial buffer
    sim808.println(cmd);
    LogManager::log(LOG_LVL_DEBUG, cmd);

    int response = readSimResponse(timeoutMs);
    if (response == 0) {
        LogManager::log(LOG_LVL_DEBUG, "Komut başariyla yürütüldü.");
    } else if (response == 1) {
        LogManager::log(LOG_LVL_ERROR, "Komut bir HATA döndürdü.");
    } else {
        LogManager::log(LOG_LVL_WARN, "Komut yaniti bilinmiyor.");
    }
    return response;
}

int Sim808Manager::readSimResponse(unsigned long timeoutMs) {
    unsigned long startTime = millis();
    char buffer[MAX_RESPONSE_LEN] = {0};
    size_t idx = 0;

    while (millis() - startTime < timeoutMs) {
        while (sim808.available()) {
            char c = sim808.read();
            if (idx < MAX_RESPONSE_LEN - 1) {
                buffer[idx++] = c;
                buffer[idx] = '\0';
            }
        }

        // Use non-blocking delay
        unsigned long waitStart = millis();
        while (millis() - waitStart < 5) {
            yield();  //Arka plan görevlerinin çalışmasına izin ver
        }
        
    }

    LogManager::log(LOG_LVL_DEBUG, buffer);

    if (strstr(buffer, "OK")) return 0;
    if (strstr(buffer, "ERROR")) return 1;
    return 2;
}
