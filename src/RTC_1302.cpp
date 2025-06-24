#include "RTC_1302.h"
#include <Config.h>
#include "LoggerModule.h"

// countof makrosu, C++'da dizi boyutunu güvenli bir şekilde almak için kullanılır.
#ifndef countof
#define countof(a) (sizeof(a) / sizeof(a[0]))
#endif

RTC_Module::RTC_Module(uint8_t io_pin, uint8_t sclk_pin, uint8_t ce_pin)
    : _myWire(io_pin, sclk_pin, ce_pin), _Rtc(_myWire) {}

void RTC_Module::begin() {
    // Serial.begin() ana main.cpp içinde çağrıldığı için burada tekrar çağrılmasına gerek yoktur.
    // Ancak, modül içinde bağımsız hata ayıklama mesajları için gerekebilir.

    Serial.println("RTC_Module: Başlatılıyor...");
    
    _Rtc.Begin();
    initializeRtc(); // RTC'yi yapılandır ve gerekirse zamanı ayarla
}

void RTC_Module::initializeRtc() {
    /*
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    Serial.print("RTC_Module: Derleme zamanı: ");
    printDateTimeToSerial(compiled);
    Serial.println();

    if (!_Rtc.IsDateTimeValid()) {
        LOG_INFO("RTC_Module: RTC DateTime geçersiz! Derleme zamanına ayarlanıyor.");
        _Rtc.SetDateTime(compiled);
    }*/

    if (_Rtc.GetIsWriteProtected()) {
        LOG_INFO("RTC_Module: RTC yazmaya karşı korumalıydı, yazma etkinleştiriliyor.");
        _Rtc.SetIsWriteProtected(false);
    }

    if (!_Rtc.GetIsRunning()) {
        LOG_INFO("RTC_Module: RTC çalışmıyordu, şimdi başlıyor.");
        _Rtc.SetIsRunning(true);
    }
    RtcDateTime now = _Rtc.GetDateTime();
    printDateTimeToSerial(now);
    /*if (now < compiled) {
        LOG_INFO("RTC derleme zamanından daha eskidir! (DateTime güncelleniyor)");
        _Rtc.SetDateTime(compiled);
    } else if (now > compiled) {
        LOG_INFO("RTC derleme zamanından daha yenidir. (bu beklenen bir durumdur)");
        printDateTimeToSerial(now);
    } else if (now == compiled) {
        LOG_INFO("RTC derleme zamanı ile aynıdır! ");
        printDateTimeToSerial(now);
    }*/
}
    

RtcDateTime RTC_Module::getDateTime() {
    return _Rtc.GetDateTime();
}

void RTC_Module::printDateTimeToSerial(const RtcDateTime& dt) {
    char datestring[25]; // "AA/GG/YYYY SS:DD:ss" formatı için yeterli alan

    snprintf_P(datestring,
               countof(datestring),
               PSTR("%02u/%02u/%04u %02u:%02u:%02u"), // PSTR program belleğinden okur
               dt.Month(),
               dt.Day(),
               dt.Year(),
               dt.Hour(),
               dt.Minute(),
               dt.Second());
    LOG_INFO(datestring);
}

String RTC_Module::getTimestamp() {
    RtcDateTime now = _Rtc.GetDateTime();
    if (!now.IsValid()) {
        return "NOT_IMPLEMENTED_TIMESTAMP";
    }
    // YYYY-MM-DD HH:MM:SS formatı için
    char timestampBuffer[20]; 
    snprintf(timestampBuffer, sizeof(timestampBuffer), "%04u-%02u-%02u %02u:%02u:%02u",
             now.Year(),
             now.Month(),
             now.Day(),
             now.Hour(),
             now.Minute(),
             now.Second());
    return String(timestampBuffer);
}

void RTC_Module::setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    RtcDateTime newDateTime(year, month, day, hour, minute, second);
    if (newDateTime.IsValid()) {
        _Rtc.SetDateTime(newDateTime);
        LOG_INFO("RTC zamanı başarıyla ayarlandı.");
        printDateTimeToSerial(newDateTime);
        Serial.println();
    } else {
        LOG_ERROR("RTC için ayarlanan tarih/saat geçersiz.");
    }
}