#ifndef _MODULE_SERIAL_GSM_PHONE_H
#define _MODULE_SERIAL_GSM_PHONE_H

#include "ModuleSerialGsm.h"
#include "LoggerModule.h"
#include "ErrorCodes.h"

#define CALL_NUMBER_LENGTH 30

class ModuleSerialGsm_Phone : public ModuleSerialGsm 
{
public:
    ModuleSerialGsm_Phone(ModuleSerialCore *core);

    int enable(const char *pin);

    bool callAvailable();
    void callMake(const char *number, unsigned long timeout);
    void callAnswer();
    void callDrop();

    void receivedNumber(char *output, int size);

private:
    char callNumber[CALL_NUMBER_LENGTH];

    void parseCall(char *call);
};

#endif // _MODULE_SERIAL_GSM_PHONE_H
