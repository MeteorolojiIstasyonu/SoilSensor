#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <Arduino.h>

class LogManager {
public:
    static void log(uint8_t level, const char* msg);
};

#endif // LOG_MANAGER_H
