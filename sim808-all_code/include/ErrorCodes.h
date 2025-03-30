#ifndef ERROR_CODES_H
#define ERROR_CODES_H

// Genel hata kodları (modüller arası ortak kullanım için)
#define ERR_OK                     0x00  // Başarılı                   0
#define ERR_INIT_FAIL              0x01  // Başlatma hatası            1
#define ERR_COMM_FAIL              0x02  // Haberleşme hatası          2
#define ERR_TIMEOUT                0x03  // Zaman aşımı                3
#define ERR_INVALID_RESPONSE       0x04  // Geçersiz yanıt             4
#define ERR_QUEUE_OVERFLOW         0x05  // Kuyruk doldu               5
#define ERR_MEMORY_ALLOCATION      0x06  // Bellek tahsis hatası       6
#define ERR_UNKNOWN                0xFF  // Bilinmeyen hata            255

// GPS hata kodları
#define ERR_GPS_INIT_FAIL          0x10  // GPS başlatma hatası        16
#define ERR_GPS_NO_SIGNAL          0x11  // GPS sinyali yok            17
#define ERR_GPS_PARSE_FAIL         0x12  // GPS verisi ayrıştırılamadı 18

// GSM hata kodları
#define ERR_GSM_INIT_FAIL          0x20  // GSM başlatma hatası        32
#define ERR_GSM_NO_NETWORK         0x21  // GSM şebekesi yok           33
#define ERR_GSM_SEND_FAIL          0x22  // GSM mesaj gönderme hatası  34
#define ERR_GSM_CALL_FAIL          0x23  // Arama başarısız            35

// GPRS hata kodları
#define ERR_GPRS_INIT_FAIL         0x30  // GPRS başlatma hatası       48
#define ERR_GPRS_CONNECT_FAIL      0x31  // GPRS bağlantı hatası       49
#define ERR_GPRS_HTTP_FAIL         0x32  // HTTP isteği başarısız      50

#endif // ERROR_CODES_H
