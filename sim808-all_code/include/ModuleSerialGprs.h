#ifndef _MODULE_SERIAL_GPRS_H
#define _MODULE_SERIAL_GPRS_H

#include "ModuleSerialCore.h"
#include "LoggerModule.h"
#include "ErrorCodes.h"

#define GPRS_FAIL -1 
#define GPRS_ENABLED 0

#define HTTP_GET  0
#define HTTP_POST 1
#define HTTP_HEAD 2

class ModuleSerialGprs
{
public:
    struct HttpResponse
    {
        int method;
        int statusCode;
        int contentLength;
    };

    ModuleSerialGprs(ModuleSerialCore *core);

    int enable(const char *apn, const char* username, const char *password);
    int isReady();
    void disable();

    void openHttpConnection();
    void closeHttpConnection();

    HttpResponse sendHttpRequest(int method, const char *url, unsigned long timeout);   
    void readHttpResponse(int count, char *output, int size);

    // **Yeni eklenen fonksiyonlar**
  
    int getNetworkTime(char *datetime, int size);       // NTP sunucusuyla saat senkronizasyonu

private:
    ModuleSerialCore *core = nullptr;

    void parseHttpResponse(HttpResponse *httpResponse, char *response);
    void parseReadContent(char *content);
};

#endif 
