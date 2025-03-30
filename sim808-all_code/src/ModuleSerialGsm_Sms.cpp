#include "ModuleSerialGsm_Sms.h"

ModuleSerialGsm_Sms::ModuleSerialGsm_Sms(ModuleSerialCore *core) : ModuleSerialGsm(core)
{
}

int ModuleSerialGsm_Sms::enable(const char *pin)
{
    LOG_INFO("GSM SMS modülü etkinleştiriliyor...");

    if (!ModuleSerialGsm::enterPin(pin)) 
    {
        LOG_ERROR("GSM PIN girişi başarısız! Hata kodu: %d", ERR_GSM_INIT_FAIL);
        return GSM_FAIL;
    }

    delay(2000);
    ModuleSerialGsm::registerOnNetwork();

    if (!core->writeCommand("AT+CMGF=1", "OK", 2000)) {
        LOG_ERROR("SMS formatı ayarlanamadı! Hata kodu: %d", ERR_GSM_INIT_FAIL);
        return GSM_FAIL;
    }
    if (!core->writeCommand("AT+CNMI=2,1,0,0,0", "OK", 2000)) {
        LOG_ERROR("Yeni SMS bildirimi ayarlanamadı! Hata kodu: %d", ERR_GSM_INIT_FAIL);
        return GSM_FAIL;
    }

    core->writeCommand("AT+CMGD=1,4", "OK", 2000);
    LOG_INFO("GSM SMS modülü başarıyla etkinleştirildi.");
    return GSM_ENABLED;
}

bool ModuleSerialGsm_Sms::messageAvailable()
{
    char response[200] = "";
    core->writeCommand("AT+CMGR=1", response, 200, 2000);

    if (strstr(response, "+CMGR:") != NULL)
    {
        parseMessage(response);
        LOG_INFO("Yeni SMS alındı! Gönderen: %s", messageNumber);
        return true;        
    }

    LOG_DEBUG("Yeni SMS yok.");
    return false;
}

void ModuleSerialGsm_Sms::messageSend(const char *number, const char *content)
{
    LOG_INFO("SMS gönderiliyor... Alıcı: %s", number);

    char command[40] = "";
    sprintf(command, "AT+CMGS=\"%s\"", number);

    if (!core->writeCommand(command, ">", 2000)) {
        LOG_ERROR("SMS gönderme başlatılamadı! Hata kodu: %d", ERR_GSM_SEND_FAIL);
        return;
    }

    core->getSerial().print(content);
    core->getSerial().write(0x1A); // CTRL+Z gönder

    delay(250);
    LOG_INFO("SMS başarıyla gönderildi.");
}

void ModuleSerialGsm_Sms::messageFlush()
{
    LOG_INFO("SMS kutusu temizleniyor...");
    core->writeCommand("AT+CMGD=1,3", "OK", 2000);  
}

void ModuleSerialGsm_Sms::receivedNumber(char *output, int size)
{
    int i = 0;
    while (messageNumber[i] != '\0' && i < size)
    {
        output[i] = messageNumber[i];
        ++i;
    }
    output[i] = '\0';

    LOG_DEBUG("Gelen SMS numarası: %s", output);
}

void ModuleSerialGsm_Sms::receivedContent(char *output, int size)
{
    int i = 0;
    while (messageContent[i] != '\0' && i < size)
    {
        output[i] = messageContent[i];
        ++i;
    }
    output[i] = '\0';

    LOG_DEBUG("Gelen SMS içeriği: %s", output);
}

void ModuleSerialGsm_Sms::parseMessage(char *message)
{
    memset(messageNumber, '\0', MESSAGE_NUMBER_LENGTH);
    memset(messageContent, '\0', MESSAGE_CONTENT_LENGTH);

    int i = 32;
    int j = 0;
    int k = 0;

    while (message[++i] != '+' && message[i] != '\0');
    while (message[i] != '"' && message[i] != '\0')
    {
        messageNumber[j++] = message[i++];
    }
    while (message[i++] != '\n' && message[i] != '\0');
    while (message[i] != '\n' && message[i] != '\0')
    {
        messageContent[k++] = message[i++];
    }

    LOG_DEBUG("SMS ayrıştırıldı -> Numara: %s, İçerik: %s", messageNumber, messageContent);
}
