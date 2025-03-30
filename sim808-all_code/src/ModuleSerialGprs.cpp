#include "ModuleSerialGprs.h"

ModuleSerialGprs::ModuleSerialGprs(ModuleSerialCore *core)
{
    this->core = core;
}

int ModuleSerialGprs::enable(const char *apn, const char* username, const char *password)
{
    LOG_INFO("GPRS etkinleştiriliyor...");

    if (!core->writeCommand("AT+CGATT=1", "OK", 2000)) {
        LOG_ERROR("GPRS bağlantısı başarısız! Hata kodu: %d", ERR_GPRS_INIT_FAIL);
        return GPRS_FAIL;
    }

    core->writeCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", "OK", 2000);

    char command[50] = "";
    sprintf(command, "AT+SAPBR=3,1,\"APN\",\"%s\"", apn);
    core->writeCommand(command, "OK", 2000);

    sprintf(command, "AT+SAPBR=3,1,\"USER\",\"%s\"", username);
    core->writeCommand(command, "OK", 2000);

    sprintf(command, "AT+SAPBR=3,1,\"PWD\",\"%s\"", password);
    core->writeCommand(command, "OK", 2000);

    if (core->writeCommand("AT+SAPBR=2,1", "0.0.0.0", 2000)) {
        if (!core->writeCommand("AT+SAPBR=1,1", "OK", 2000)) {
            LOG_ERROR("GPRS bağlantısı başlatılamadı! Hata kodu: %d", ERR_GPRS_CONNECT_FAIL);
            return GPRS_FAIL;
        }
    }

    LOG_INFO("GPRS başarıyla etkinleştirildi.");
    return GPRS_ENABLED;
}

int ModuleSerialGprs::isReady()
{
    bool status = core->writeCommand("AT+CGATT=1", "OK", 2000);
    LOG_DEBUG("GPRS bağlantısı aktif mi? %s", status ? "EVET" : "HAYIR");
    return status ? GPRS_ENABLED : GPRS_FAIL;
}

void ModuleSerialGprs::disable()
{
    LOG_INFO("GPRS devre dışı bırakılıyor...");
    core->writeCommand("AT+CGATT=0", "+SAPBR 1: DEACT", 2000);
}

void ModuleSerialGprs::openHttpConnection()
{
    LOG_INFO("HTTP bağlantısı başlatılıyor...");
    core->writeCommand("AT+HTTPINIT", "OK", 2000);
}

void ModuleSerialGprs::closeHttpConnection()
{
    LOG_INFO("HTTP bağlantısı sonlandırılıyor...");

    char responsee[50]; // Yanıtı saklamak için buffer (100 byte)
    
    core->writeCommand("AT+HTTPTERM", responsee, sizeof(responsee), 2000);

    LOG_INFO("Modülden gelen yanıt: %s", responsee);
}


ModuleSerialGprs::HttpResponse ModuleSerialGprs::sendHttpRequest(int method, const char *url, unsigned long timeout)
{
    LOG_INFO("HTTP isteği gönderiliyor... Metod: %d, URL: %s", method, url);
    
    core->writeCommand("AT+HTTPPARA=\"CID\",1", "OK", 2000);

    char command[200] = "";
    sprintf(command, "AT+HTTPPARA=\"URL\",\"%s\"", url);
    core->writeCommand(command, "OK", 2000);

    char response[50] = "";
    sprintf(command, "AT+HTTPACTION=%d", method);
    core->writeCommand(command, response, 50, timeout);

    ModuleSerialGprs::HttpResponse httpResponse;

    if (strstr(response, "+HTTPACTION:") != NULL) {
        parseHttpResponse(&httpResponse, response);
        LOG_INFO("HTTP yanıt kodu: %d, İçerik uzunluğu: %d", httpResponse.statusCode, httpResponse.contentLength);
    } else {
        LOG_ERROR("HTTP isteği başarısız! Hata kodu: %d", ERR_GPRS_HTTP_FAIL);
    }

    return httpResponse;
}

void ModuleSerialGprs::readHttpResponse(int count, char *output, int size)
{
    LOG_INFO("HTTP yanıtı okunuyor, %d bayt...", count);

    char command[50] = "";
    sprintf(command, "AT+HTTPREAD=0,%d", count);
    core->writeCommand(command, output, size, 2000);

    if (strstr(output, "+HTTPREAD:") != NULL) {
        parseReadContent(output);
    }
}

void ModuleSerialGprs::parseHttpResponse(ModuleSerialGprs::HttpResponse *httpResponse, char *response)
{
    char *pch = strtok(response, " ,");
    pch = strtok(NULL, " ,"); httpResponse->method = atoi(pch);
    pch = strtok(NULL, " ,"); httpResponse->statusCode = atoi(pch);
    pch = strtok(NULL, " ,"); httpResponse->contentLength = atoi(pch);

    LOG_DEBUG("HTTP Yanıt Ayrıştırıldı -> Metod: %d, Kod: %d, İçerik Uzunluğu: %d",
              httpResponse->method, httpResponse->statusCode, httpResponse->contentLength);
}

void ModuleSerialGprs::parseReadContent(char *content)
{
    char *pch = strtok(content, "\n");
    pch = strtok(NULL, "\n");
    pch = strtok(NULL, "\n");
    strcpy(content, pch);

    LOG_DEBUG("HTTP Yanıt İçeriği: %s", content);
}

/*** NTP ile Saat Senkronizasyonu ve Düzeltme ***/
int ModuleSerialGprs::getNetworkTime(char *datetime, int size) {
    LOG_INFO("NTP sunucusundan saat ve tarih alınıyor...");
    
    if (!core->writeCommand("AT+CNTPCID=1", "OK", 2000)) {
        LOG_ERROR("NTP bağlantısı başarısız!");
        return GPRS_FAIL;
    }
    
    if (!core->writeCommand("AT+CNTP=\"pool.ntp.org\",0", "OK", 5000)) {
        LOG_ERROR("NTP sunucusuna bağlanılamadı!");
        return GPRS_FAIL;
    }
    
    char response[50] = {0};  // AT cevabını saklamak için değişken
    core->writeCommand("AT+CCLK?", response, sizeof(response), 2000);
    
    if (strstr(response, "+CCLK:") != NULL) {
        // +CCLK cevabını parse etme
        char *timeString = strstr(response, "+CCLK: ");
        if (timeString != NULL) {
            timeString += 7; // "+CCLK: " kısmını atla
            
            // Zaman bilgisini ayrıştırma
            struct tm timeinfo;
            int year, month, day, hour, min, sec;
            sscanf(timeString, "\"%d/%d/%d,%d:%d:%d", 
                   &year, &month, &day, &hour, &min, &sec);
            
            // timeinfo yapısını doldurma
            timeinfo.tm_year = 2000 + year - 1900; // Yıl düzeltmesi
            timeinfo.tm_mon = month - 1;          // Ay 0-11 arası
            timeinfo.tm_mday = day;
            timeinfo.tm_hour = hour;
            timeinfo.tm_min = min;
            timeinfo.tm_sec = sec;
            
            // Zaman farkını hesaplama 
            const int yearDiff = 2025 - 2004;  // 21 yıl fark
            const int monthDiff = 3 - 1;      // 2 ay fark
            const int dayDiff = 28 - 1;       // 21 gün fark
            const int hourDiff = 19 - 2;      // 17 saat fark
            const int minDiff = 43 - 2;      // 6 dakika fark
            const int secDiff = 42 - 42;      // 0 saniye fark
            
            // Düzeltilmiş tarihi hesaplama
            struct tm correctedTime = timeinfo;
            correctedTime.tm_year += yearDiff;
            correctedTime.tm_mon += monthDiff;
            correctedTime.tm_mday += dayDiff;
            correctedTime.tm_hour += hourDiff;
            correctedTime.tm_min += minDiff;
            correctedTime.tm_sec += secDiff;
            
            // Taşmaları düzeltme için mktime kullanma
            time_t t = mktime(&correctedTime);
            struct tm *adjustedTime = localtime(&t);
            
            // İstenen formatta düzeltilmiş tarihi oluşturma
            char currentDateTime[32];
            snprintf(currentDateTime, sizeof(currentDateTime), "%02d/%02d/%04d %02d:%02d:%02d",
                  adjustedTime->tm_mday, adjustedTime->tm_mon + 1, adjustedTime->tm_year + 1900,
                  adjustedTime->tm_hour, adjustedTime->tm_min, adjustedTime->tm_sec);
            
            // Sonucu çıktı parametresine kopyalama
            strncpy(datetime, currentDateTime, size - 1);
            datetime[size - 1] = '\0';  // Güvenlik için son karakteri NULL yap
            
            LOG_INFO("Düzeltilmiş tarih ve saat: %s", datetime);
            return GPRS_ENABLED;
        }
    }
    
    LOG_ERROR("Saat ve tarih alınamadı!");
    return GPRS_FAIL;
}