#include "ModuleSerialGsm.h"

ModuleSerialGsm::ModuleSerialGsm(ModuleSerialCore *core)
{
    this->core = core;
}

ModuleSerialGsm::BatteryStatus ModuleSerialGsm::currentBatteryStatus()
{
    LOG_INFO("Batarya durumu sorgulanıyor...");
    
    char response[50] = "";
    core->writeCommand("AT+CBC", response, 50, 2000);

    BatteryStatus batteryStatus = {0};

    if (strstr(response, "+CBC:") != NULL)
    {
        parseBatteryStatus(&batteryStatus, response);
        LOG_INFO("Batarya durumu -> Mod: %d, Kapasite: %d%%, Voltaj: %dmV", 
                 batteryStatus.mode, batteryStatus.capacity, batteryStatus.voltage);
    }
    else
    {
        LOG_ERROR("Batarya bilgisi alınamadı! Hata kodu: %d", ERR_GSM_INIT_FAIL);
    }

    return batteryStatus;
}

int ModuleSerialGsm::currentSignalQuality()
{
    LOG_INFO("Sinyal kalitesi sorgulanıyor...");
    
    char response[30] = "";
    core->writeCommand("AT+CSQ", response, 30, 2000);

    int signalQuality = 0;

    if (strstr(response, "+CSQ:") != NULL)
    {
        parseSignalQuality(&signalQuality, response);
        LOG_INFO("Sinyal kalitesi: %d", signalQuality);
    }
    else
    {
        LOG_ERROR("Sinyal bilgisi alınamadı! Hata kodu: %d", ERR_GSM_NO_NETWORK);
    }

    return signalQuality;
}

void ModuleSerialGsm::currentNetwork(char *output)
{
    LOG_INFO("Ağ bilgisi sorgulanıyor...");
    
    char response[50] = "";
    core->writeCommand("AT+COPS?", response, 50, 2000);

    if (strstr(response, "+COPS:") != NULL)
    {
        parseNetwork(output, response);
        LOG_INFO("Bağlı şebeke: %s", output);
    }
    else
    {
        LOG_ERROR("Ağ bilgisi alınamadı! Hata kodu: %d", ERR_GSM_NO_NETWORK);
    }
}

bool ModuleSerialGsm::enterPin(const char *pin)
{
    LOG_INFO("GSM PIN doğrulanıyor...");

    if (!core->writeCommand("AT+CPIN?", "READY", 2000))
    {
        char command[25] = "";
        sprintf(command, "AT+CPIN=\"%s\"", pin);

        if (!core->writeCommand(command, "OK", 2000))
        {
            LOG_ERROR("PIN doğrulama başarısız! Hata kodu: %d", ERR_GSM_INIT_FAIL);
            return false;
        }
    }

    LOG_INFO("GSM PIN doğrulandı.");
    return true;
}

void ModuleSerialGsm::registerOnNetwork()
{
    LOG_INFO("Şebekeye kayıt olunuyor...");

    while (!core->writeCommand("AT+CREG?", "+CREG: 0,1", 2000) &&
           !core->writeCommand("AT+CREG?", "+CREG: 0,5", 2000))
    {
        LOG_WARN("Şebekeye bağlanılamadı, tekrar deneniyor...");
        delay(2000);
    }

    LOG_INFO("GSM şebekesine başarıyla bağlanıldı.");
}

void ModuleSerialGsm::parseBatteryStatus(BatteryStatus *batteryStatus, char *response)
{
    char *pch = strtok(response, " ,");
    pch = strtok(NULL, " ,"); batteryStatus->mode = atoi(pch);
    pch = strtok(NULL, " ,"); batteryStatus->capacity = atoi(pch);
    pch = strtok(NULL, " ,"); batteryStatus->voltage = atoi(pch);
}

void ModuleSerialGsm::parseSignalQuality(int *signalQuality, char *response)
{
    char *pch = strtok(response, " ,");
    pch = strtok(NULL, " ,");
    *signalQuality = atoi(pch);
}

void ModuleSerialGsm::parseNetwork(char *network, char *response)
{
    char *pch = strtok(response, " ,");
    pch = strtok(NULL, " ,");
    pch = strtok(NULL, " ,");
    pch = strtok(NULL, "\n");
    strcpy(network, pch);
    network[strlen(network) - 1] = '\0';
}
