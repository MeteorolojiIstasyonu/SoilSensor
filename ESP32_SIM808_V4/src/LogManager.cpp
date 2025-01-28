#include "LogManager.h"
#include "Config.h"

void LogManager::log(uint8_t level, const char* msg) {
    if (level > CURRENT_LOG_LEVEL) {
        return;
    }
    switch (level) {
        case LOG_LVL_ERROR: Serial.print("[ERROR] "); break;
        case LOG_LVL_WARN:  Serial.print("[WARN ] "); break;
        case LOG_LVL_INFO:  Serial.print("[INFO ] "); break;
        case LOG_LVL_DEBUG: Serial.print("[DEBUG] "); break;
        default:            Serial.print("[???? ] "); break;
    }
    Serial.println(msg);
}
