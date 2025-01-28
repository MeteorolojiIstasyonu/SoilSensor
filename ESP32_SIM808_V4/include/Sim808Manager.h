#ifndef SIM808_MANAGER_H
#define SIM808_MANAGER_H

#include <Arduino.h>

class Sim808Manager {
public:
    static void initializeSIM808();
    static bool connectToGPRS();
    static void checkGPRSConnection();
    static int sendATCommand(const char* cmd, unsigned long timeoutMs);
    static int readSimResponse(unsigned long timeoutMs);
};

#endif // SIM808_MANAGER_H
