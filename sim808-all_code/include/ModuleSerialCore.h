#ifndef _MODULE_SERIAL_CORE_H
#define _MODULE_SERIAL_CORE_H

#include <Arduino.h>
#include "LoggerModule.h"
#include "ErrorCodes.h"

#define MODULE_FAIL  -1
#define MODULE_READY  0

class ModuleSerialCore 
{
public:
    ModuleSerialCore(HardwareSerial &serialPort, int rxPin, int txPin);

    void debug(HardwareSerial *printer);
    int begin(int baudRate);
    int isReady();

    bool writeCommand(const char *command, const char *expected, unsigned long timeout);
    void writeCommand(const char *command, char *output, int size, unsigned long timeout);

    // Seri portu döndüren getter fonksiyonu
    HardwareSerial& getSerial() { return serial; }
    
protected:
    HardwareSerial &serial;
    HardwareSerial *printer = nullptr;
};

#endif  
