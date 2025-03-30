#include "ModuleSerialGsm_Phone.h"

ModuleSerialGsm_Phone::ModuleSerialGsm_Phone(ModuleSerialCore *core) : ModuleSerialGsm(core)
{
}

int ModuleSerialGsm_Phone::enable(const char *pin)
{
    LOG_INFO("GSM Telefon modülü etkinleştiriliyor...");

    if (!ModuleSerialGsm::enterPin(pin)) 
    {
        LOG_ERROR("GSM PIN girişi başarısız! Hata kodu: %d", ERR_GSM_INIT_FAIL);
        return GSM_FAIL;
    }

    delay(2000);
    ModuleSerialGsm::registerOnNetwork();

    LOG_INFO("GSM Telefon modülü başarıyla etkinleştirildi.");
    return GSM_ENABLED;
}

bool ModuleSerialGsm_Phone::callAvailable()
{
    char command[100] = "";
    int i = 0;

    while (ModuleSerialGsm::core->getSerial().available())  
    {
        command[i++] = ModuleSerialGsm::core->getSerial().read();
        if (i >= 99) 
        {
            command[i] = '\0';
            LOG_WARN("Çağrı algılama tamponu doldu!");
            return false;
        }
    }

    if (strstr(command, "RING"))
    {
        parseCall(command); 
        LOG_INFO("Gelen çağrı tespit edildi: %s", callNumber);
        return true;
    }

    return false;
}

void ModuleSerialGsm_Phone::callMake(const char *number, unsigned long timeout)
{
    LOG_INFO("Arama yapılıyor: %s", number);

    char command[40] = "";
    sprintf(command, "ATD%s;", number);

    if (!core->writeCommand(command, "+COLP:", timeout))
    {
        LOG_ERROR("Arama başarısız! Hata kodu: %d", ERR_GSM_CALL_FAIL);
        callDrop();
    }

    delay(250);
}

void ModuleSerialGsm_Phone::callAnswer()
{
    LOG_INFO("Gelen çağrı cevaplanıyor...");
    core->writeCommand("ATA", "OK", 2000);
}

void ModuleSerialGsm_Phone::callDrop()
{
    LOG_INFO("Çağrı sonlandırılıyor...");
    core->writeCommand("ATH", "OK", 2000);
}

void ModuleSerialGsm_Phone::receivedNumber(char *output, int size)
{
    int i = 0;
    while (callNumber[i] != '\0' && i < size)
    {
        output[i] = callNumber[i];
        ++i;
    }
    output[i] = '\0';

    LOG_DEBUG("Çağrı yapan numara: %s", output);
}

void ModuleSerialGsm_Phone::parseCall(char *call)
{
    memset(callNumber, '\0', CALL_NUMBER_LENGTH);

    int i = 12;
    int j = 0;
    
    while (call[++i] != '+' && call[i] != '\0');
    while (call[i] != '"' && call[i] != '\0')
    {
        callNumber[j++] = call[i++];
    }

    LOG_DEBUG("Çağrı numarası ayrıştırıldı: %s", callNumber);
}
