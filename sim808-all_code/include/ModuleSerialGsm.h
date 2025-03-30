#ifndef _MODULE_SERIAL_GSM_H
#define _MODULE_SERIAL_GSM_H

#include "ModuleSerialCore.h"
#include "LoggerModule.h"
#include "ErrorCodes.h"

#define GSM_FAIL -1 
#define GSM_ENABLED 0

class ModuleSerialGsm
{
public:
    struct BatteryStatus
    {
        int mode;       // 0 => şarj olmuyor, 1 => şarj oluyor, 2 => şarj tamamlandı
        int capacity;   // Batarya yüzdesi (%)
        int voltage;    // Batarya voltajı (mV)
    };

    ModuleSerialGsm(ModuleSerialCore *core);

    BatteryStatus currentBatteryStatus();
    int currentSignalQuality();
    void currentNetwork(char *output);

protected:
    ModuleSerialCore *core = nullptr;

    bool enterPin(const char *pin);
    void registerOnNetwork();

    void parseBatteryStatus(BatteryStatus *batteryStatus, char *response);
    void parseSignalQuality(int *signalQuality, char *response);
    void parseNetwork(char *network, char *response);
};

#endif // _MODULE_SERIAL_GSM_H
