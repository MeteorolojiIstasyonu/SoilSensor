#include "Communication_Driver.h"
#include <ArduinoJson.h> 
#include <Arduino.h>
#include "Config.h"
#include "RTC_1302.h" 
#include "sd_card.h" // SD kart fonksiyonları için
#include "FS.h"      // SD kart için
#include "SD.h"      // SD kart için

extern String deviceId;

static const char* aws_root_ca_pem_driver PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char* aws_certificate_pem_driver PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
MIIDWjCCAkKgAwIBAgIVAIym4XA5VZ4sD6TDViW06ZHKU3T1MA0GCSqGSIb3DQEB
CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t
IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0yNTAzMDIyMTQ1
MTZaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh
dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDUwX0YI0wCyHhhDaQf
f00/9pD+dTvLW0t7P4SpdTN9uEtk4LCLx6O11OqMK6tDvnulCgLkEIw9Hp+ij5tO
IKbquxreZaIawy/E4D9XKu8fi9YA3NtXcgr6qQ+VlUYvuftTB5LQexL3cjZdOINL
jQY7wwTHBL3vOAwKlpzc0PRhMAmNPX0CWYEs7IgFTl8Epf/AKQKQyhWhOAWyywIK
7OJ4SYsfHqTs/qR99YU88RTouzpnQ0C2hkmZNhaTmdTQ7UpX6IZ2Pww3zhMopOEU
MGXiPY10tLeUAgkSLkJcMDHimBlTMfhZfxQr1DBiXCZV0oob2cYZuKySSb9/m6Zm
4GArAgMBAAGjYDBeMB8GA1UdIwQYMBaAFBJgcsWujBlqdsrDddsh8Kh21H6dMB0G
A1UdDgQWBBSEyUI0vZcHVNXM5Vhtmjb54s7BmzAMBgNVHRMBAf8EAjAAMA4GA1Ud
DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAz1QGqf6nDtjp7Rt9c/lRv7ad
IUDUTfTzA9Ou6VxzKQCAj+KWpeU9H0A9C2/LMgn7yMaLVFZH+Xo2GpEoNqcYWzJf
vY83pu/ZItbiFQdvL8cdbntRGB8sXNc8rL3MUkmjjNfCi1i0Eq0gZ7AcnvWST1hv
3pyCg4vZ0K8rCwAaRxWzgbJSYKSoctgmMTG8TFTE/IDJ3B5D0DST+Pb0MmfxipYu
ytEsk0OSuQRkRmHnSrsCXgcRb07kxJZ0KhuHZiT5c0wyZbIdx08PNrfFU+yd3+or
AtGGpOBRST2aC+xl48lZXztuYvtDLgBaMMwsaV6px3oPszyzE5X7+PU7TFniiQ==
-----END CERTIFICATE-----
)KEY";

static const char* aws_private_pem_driver PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA1MF9GCNMAsh4YQ2kH39NP/aQ/nU7y1tLez+EqXUzfbhLZOCw
i8ejtdTqjCurQ757pQoC5BCMPR6foo+bTiCm6rsa3mWiGsMvxOA/VyrvH4vWANzb
V3IK+qkPlZVGL7n7UweS0HsS93I2XTiDS40GO8MExwS97zgMCpac3ND0YTAJjT19
AlmBLOyIBU5fBKX/wCkCkMoVoTgFsssCCuzieEmLHx6k7P6kffWFPPEU6Ls6Z0NA
toZJmTYWk5nU0O1KV+iGdj8MN84TKKThFDBl4j2NdLS3lAIJEi5CXDAx4pgZUzH4
WX8UK9QwYlwmVdKKG9nGGbiskkm/f5umZuBgKwIDAQABAoIBAHJoYgaa5IMSnnlC
RqGRaU8eHjZXgIIIY/yw2XvuxHO0qQZkNUvVXVmoV0BtMznIsuC7E3bk1yT+1MUs
CE3pDRlo6Dfz20oc8BEkrasIMXJ7Vec83M6XSwQj6Xd8wDNmBZpOlkp6BGcACe/z
NddozJNSeb0z9ZcwQnlnKI8t5lxj5xT8JSDWfMBQGB+1xEJ4cg+Gj/Qz9VplUcIQ
K/dQFGOuvC6wpXXygfLiMbqdHGgyJbJJenogZ6KJAL8dbDWXaIlY5oQx1rz7Mi5M
mizWmNYfdj23Mvh6ZbbJ4GaMuxodzl03HvCM66WL4Okhp1a+rEbJraah+cNgWTg8
fThH3fECgYEA+TTo3dTzUoIK4mW/oHde3ApR2jHq7K7IzIHufw+s8/0EQUc0jmKK
V4+a1clIsG6PrFwqvjlwOL6w+CgNLQ2DyFeDP5MrKWFJhinvGKTi8kMf1uPXAH5g
5uNMr/3YrvAvSVN3h8t3sxTLeFyZouBcKgWqkKiAa707khs0qt8zzTUCgYEA2o40
2RRMHzByYzAGqN/K4GqQ1LKVWIYmyPrrQenq17BKRcebmyYyK1T61sJOkn4pTcSN
nYc/3W6HIQFY/4yk1viVZSBprrDJ6gH1bVyvuOLqqJ/3AO7qpPifIEL2b+iu+tW+
6ceiQxm9X2h9I2kZULhGZHev/btIHHK6p2gVA98CgYEAqaKFfTNO6nQQ+qluNsnq
7Xes3g0qsDAOCX/Mm/tMrM0nT1QsB1w2dYIQUMRyUX8BF7+pbNFmfYn4pwOEbI2N
jhtcATOppsJNrSDwW2MqBOUCUGHJYdGlHqXM9uOh0vs2BQDnFa2/7kwScPz/q+pz
cjtnLo8006H9YehZApNrDJ0CgYBcyr3TYNPE9jvKswxQzNuFFpmxRLU15Zc4A5i4
3ojv1JBkOhBt+fSZAzaQ0eSsO9Zrh0UdGdxatl+2+qx/q4YdI2PCkNVt7u97ZCOA
sDaHSAibWXd0tPt42XouJ2AcOW15YCDzfuf8l0QY6vMegrPV2rdAVrSpBMfkFC39
f6pUfwKBgCMKkRTEeigTpxy9Jrg2vZoCoSXGBSDBGndWFEyRCku1JYpaTB84oo5Q
vTkjwOl7EBnWiBGTw2i37+XAfz3ykRi4KeJNRBFWJZRfAdjyKY+VqO1vIbE7Giew
hu/2rmLpS8LPoRkI7behkRxfxgDCgYLSM9KadrCOQy4qcS5OJxf/
-----END RSA PRIVATE KEY-----
)KEY";

// Initialize static member
Communication_Driver* Communication_Driver::_instance = nullptr;

Communication_Driver::Communication_Driver(HardwareSerial& modemSerialPort, NpkSensor& npkSensor, BME280Sensor& bmeSensor, RTC_Module& rtcModule)
    : _modemSerial(modemSerialPort), 
      _npkSensor(npkSensor), 
      _bmeSensor(bmeSensor),
      _rtc(rtcModule),
      _modem(_modemSerial), // TinyGsm'yi seri portla başlat
      _gsmClient(_modem),   // TinyGsmClient'ı modemle başlat
      _secureClient(&_gsmClient), // SSLClient'ı TinyGsmClient ile başlatın
      _mqttClient(_secureClient),  // PubSubClient'ı SSLClient ile başlatın
      _gps_lat(0.0f),       // GPS değişkenlerini başlat
      _gps_lon(0.0f),
      _gps_speed(0.0f),
      _gps_alt(0.0f),
      _gps_year(0),
      _gps_month(0),
      _gps_day(0),
      _gps_hour(0),
      _gps_minute(0),
      _gps_second(0),
      _gps_fix_available(false),
      _sup_bat_external(0.0f), 
      _sup_solar_external(0.0f),
      _batteryVoltageAvg(5), // Batarya voltajı için 5 ölçümlük ortalama
      _solarVoltageAvg(5)    // Solar panel voltajı için 5 ölçümlük ortalama
{
    _instance = this; // Statik örnek işaretçisini bu nesneye ayarlayın
    memset(_sup_4v, 0, sizeof(_sup_4v)); // Modem pil voltajı dizesini başlat
    _batteryVoltageAvg.clear(); // Ortalamayı başlatırken temizle
    _solarVoltageAvg.clear();   // Ortalamayı başlatırken temizle
}
bool Communication_Driver::pwrmodem() {
    // Modemi PWR_sim808 aktif hale getir
    digitalWrite(PWR_sim808, LOW); 
    delay(1000); 
    digitalWrite(PWR_sim808, HIGH);  

    int retry = 0;
    const int max_retries = 5;
    while (!_modem.testAT(1000U) && retry < max_retries) {
        LOG_WARN("AT komutuna yanıt yok, tekrar deneniyor... (%d)", retry + 1);
        digitalWrite(PWR_sim808, LOW); 
        delay(1000); 
        digitalWrite(PWR_sim808, HIGH); 
        delay(1500);
        retry++;
    }

    if (retry >= max_retries) {
        LOG_ERROR("Modem %d denemeden sonra yanıt vermiyor!", max_retries);
        return false; 
    }
    LOG_INFO("Modem AT yanıtı alındı.");

    return true; // Başarılı
}
bool Communication_Driver::setupModem() {
    LOG_INFO("Modem başlatılıyor...");
    
    if (!pwrmodem()) {
        LOG_ERROR("Modem güç kontrolü başarısız!");
        return false; 
    }

    // SIM card PIN control
    if (strlen(SIM_CARD_PIN) > 0) {
        LOG_INFO("SIM PIN giriliyor...");
        if (!_modem.simUnlock(SIM_CARD_PIN)) {
            LOG_ERROR("SIM PIN kilidi açılamadı!");
            return false; 
        }
        LOG_INFO("SIM PIN başarıyla girildi.");
    }
    // Check SIM card status
    SimStatus simStatus = _modem.getSimStatus();
    int retry = 0; 
    while (simStatus != SIM_READY && retry < 5) { 
        LOG_WARN("SIM Kart hazır değil: %d, bekleniyor...", simStatus);
        delay(1000);
        simStatus = _modem.getSimStatus();
        retry++;
    }
    if (simStatus != SIM_READY) {
        LOG_ERROR("SIM Kart %d denemeden sonra hazır değil!", retry);
        return false;
    }
    LOG_INFO("SIM Kart hazır.");
    LOG_INFO("Modem ayarları tamamlandı.");
    return true;
}

int Communication_Driver::restartModem() {
    LOG_INFO("Modem yeniden başlatılıyor...");

    // 1. Modem power down (using power control pin)
    LOG_INFO("Modem kapatılıyor (PWR_sim808 pini LOW)...");
    _modem.poweroff(); // Modemi kapatın (SIM800 serisi için poweroff())
    delay(3000); // Modemin kapanması için zaman verin

    LOG_INFO("Modem açılıyor (PWR_sim808 pini HIGH)...");
    digitalWrite(PWR_sim808, LOW); 
    delay(1000); 
    digitalWrite(PWR_sim808, HIGH); 
    delay(2000);

    
    LOG_INFO("Modem yeniden başlatılıyor ve kuruluyor...");
    if (this->setupModem()) {
        LOG_INFO("Modem başarıyla yeniden başlatıldı ve kuruldu.");
        return 0; // Success
    } else {
        LOG_ERROR("Modem yeniden başlatma ve kurulumu başarısız!");
        return 1; // Failure
    }
}

bool Communication_Driver::enableGPS() {

    setupModem();
    LOG_INFO("GPS etkinleştiriliyor...");
    if (_modem.enableGPS()) { // TinyGsmGPS.tpp içindeki public enableGPS() kullanılır
        LOG_INFO("GPS başarıyla etkinleştirildi.");
        return true;
    } else {
        LOG_ERROR("GPS etkinleştirilemedi.");
        return false;
    }
}

bool Communication_Driver::disableGPS() {
    LOG_INFO("GPS devre dışı bırakılıyor...");
    if (_modem.disableGPS()) { // TinyGsmGPS.tpp içindeki public disableGPS() kullanılır
        LOG_INFO("GPS başarıyla devre dışı bırakıldı.");
        return true;
    } else {
        LOG_ERROR("GPS devre dışı bırakılamadı.");
        return false;
    }
}

bool Communication_Driver::getGPS(float* lat, float* lon, float* speed, float* alt, int* year, int* month, int* day, int* hour, int* minute, int* second) {
    LOG_INFO("GPS konumu alınıyor...");
    // Geçici değişkenler kullanarak doğrudan sınıf üyelerine yazmaktan kaçının
    // eğer getGPS başarısız olursa eski değerler korunur.
    float temp_lat, temp_lon, temp_speed, temp_alt;
    int temp_year, temp_month, temp_day, temp_hour, temp_minute, temp_sec;

    // TinyGsmGPS.tpp içindeki public getGPS() kullanılır
    _gps_fix_available = _modem.getGPS(&temp_lat, &temp_lon, &temp_speed, &temp_alt, nullptr, nullptr, nullptr, &temp_year, &temp_month, &temp_day, &temp_hour, &temp_minute, &temp_sec);

    if (_gps_fix_available) {
        // Sınıf üyelerini güncelle
        _gps_lat = temp_lat;
        _gps_lon = temp_lon;
        _gps_speed = temp_speed;
        _gps_alt = temp_alt;
        _gps_year = temp_year;
        _gps_month = temp_month;
        _gps_day = temp_day;
        _gps_hour = temp_hour;
        _gps_minute = temp_minute;
        _gps_second = temp_sec;

        // Fonksiyon argümanlarını (pointer ise) güncelle
        if (lat) *lat = _gps_lat;
        if (lon) *lon = _gps_lon;
        if (speed) *speed = _gps_speed;
        if (alt) *alt = _gps_alt;
        if (year) *year = _gps_year;
        if (month) *month = _gps_month;
        if (day) *day = _gps_day;
        if (hour) *hour = _gps_hour;
        if (minute) *minute = _gps_minute;
        if (second) *second = _gps_second;

        snprintf(Location, sizeof(Location), "%.6f,%.6f", _gps_lat, _gps_lon);
         
        snprintf(gpsTime, sizeof(gpsTime), "%04d-%02d-%02d %02d:%02d:%02d",
            _gps_year, _gps_month, _gps_day,
            _gps_hour, _gps_minute, _gps_second);
        LOG_INFO("GPS Konumu: %s", Location);
        LOG_INFO("GPS Tarih/Saat: %s", gpsTime);
        return true;
    } else {
        LOG_WARN("GPS konumu alınamadı veya fix yok.");
        // İsteğe bağlı olarak, pointer'ları varsayılan değerlere ayarlayın
        if (lat) *lat = 0.0f;
        if (lon) *lon = 0.0f;
        // ... diğerleri için de
        return false;
    }
}

bool Communication_Driver::readGPSWithRetry(int maxRetries) {
    int retryCount = 0;
    while (retryCount < maxRetries) {
        if (getGPS(&_gps_lat, &_gps_lon, &_gps_speed, &_gps_alt, &_gps_year, &_gps_month, &_gps_day, &_gps_hour, &_gps_minute, &_gps_second)) {
            LOG_INFO("GPS Konumu alındı: Lat=%.6f, Lon=%.6f", _gps_lat, _gps_lon);
            return true; // Başarılı
        } else {
            LOG_WARN("GPS konumu alınamadı. Deneme %d/%d", retryCount + 1, maxRetries);
            delay(2000); // Bekleme süresi
            retryCount++;
        }
    }
    return false; // Tüm denemeler başarısız oldu
}
bool Communication_Driver::connectGPRS() {
    if (_modem.isGprsConnected()) {
        LOG_INFO("GPRS zaten bağlı.");
        return true;
    }

    LOG_INFO("GPRS bağlantısı kuruluyor...");
    LOG_INFO("Ağa bağlanılıyor...");
    if (!_modem.waitForNetwork(60000L)) { // Ağ kaydı için 1 dakikaya kadar bekleyin
        LOG_ERROR("Ağ bağlantısı zaman aşımına uğradı!");
        // burada modemi yeniden kurmayı deneyin veya sistemin yeniden başlat
        setupModem(); 
        return false;
    }
    LOG_INFO("Ağ bağlantısı OK. Sinyal kalitesi: %d", _modem.getSignalQuality());

    LOG_INFO("GPRS'e bağlanılıyor (APN: %s)...", APN);
    if (!_modem.gprsConnect(APN, APN_USERNAME, APN_PASSWORD)) {
        LOG_ERROR("GPRS bağlantısı başarısız!");
        delay(5000); // Tekrar denemeden veya başarısız olmadan önce bekleyin
        return false;
    }
    LOG_INFO("GPRS Bağlantısı OK. IP: %s", _modem.getLocalIP().c_str());
    return true;
}

bool Communication_Driver::connectMQTT() {
    LOG_INFO("Modem başlatılıyor...");
    connectGPRS(); // 🔹 **GPRS Bağlantısını Kur**
const int MAX_MODEM_RETRIES = 5;
int modemRetries = 0;

while ((!_modem.isNetworkConnected() || !_modem.isGprsConnected()) && modemRetries < MAX_MODEM_RETRIES) {
    LOG_WARN("Modem veya GPRS bağlı değil. (%d/%d) Kurulum ve bağlantı yeniden deneniyor...", modemRetries + 1, MAX_MODEM_RETRIES);

    if (!setupModem()) {
        LOG_ERROR("Modem kurulumu başarısız (deneme %d)", modemRetries + 1);
    } else if (!connectGPRS()) {
        LOG_ERROR("GPRS bağlantısı başarısız (deneme %d)", modemRetries + 1);
    } else {
        break; // Başarılı bağlantı
    }

    modemRetries++;
    delay(2000); // Gecikme ile modem stabilize olabilir
}

if (!_modem.isNetworkConnected() || !_modem.isGprsConnected()) {
    LOG_ERROR("Modem/GPRS bağlantısı %d denemeden sonra başarısız oldu.", MAX_MODEM_RETRIES);
    return false;
}
    
    LOG_INFO("SSL İstemcisi ayarlanıyor...");
    _secureClient.setCACert(aws_root_ca_pem_driver);
    _secureClient.setCertificate(aws_certificate_pem_driver);
    _secureClient.setPrivateKey(aws_private_pem_driver);

    // 🔹 **MQTT Sunucusunu ve Portunu Ayarla**
    LOG_INFO("MQTT bağlantısı kuruluyor...");
    _mqttClient.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
    LOG_INFO("MQTT sunucusuna bağlanılıyor: %s:%d", AWS_IOT_ENDPOINT, AWS_IOT_PORT);
    _mqttClient.setCallback(Communication_Driver::staticMqttCallback); 

    int retries = 0;
    while (!_mqttClient.connected() && retries < 5) { // 5 kez bağlanmayı deneyin
        LOG_INFO("MQTT Bağlantı denemesi %d...", retries + 1);
        
         // Client ID ile bağlan
        if (_mqttClient.connect(MQTT_CLIENT_ID)) {
            LOG_INFO("MQTT Bağlandı!");
            // Subscribe to the topic
            if (_mqttClient.subscribe(AWS_IOT_SUBSCRIBE_TOPIC)) {
                LOG_INFO("MQTT konusuna abone olundu: %s", AWS_IOT_SUBSCRIBE_TOPIC);
            } else {
                LOG_ERROR("MQTT konusuna abone olma başarısız!");
               // Abonelik şimdilik başarısız olsa bile devam et veya false döndür
            }
            return true; // Başarıyla bağlandı
        } 
        else {
            LOG_ERROR("MQTT bağlantı hatası, rc=%d. Detaylar için PubSubClient dokümantasyonuna bakın.", _mqttClient.state());
            // Common error codes:
            // -4: MQTT_CONNECTION_TIMEOUT
            // -3: MQTT_CONNECTION_LOST
            // -2: MQTT_CONNECT_FAILED
            // -1: MQTT_DISCONNECTED
            //  1: MQTT_CONNECT_BAD_PROTOCOL
            //  2: MQTT_CONNECT_BAD_CLIENT_ID
            //  3: MQTT_CONNECT_UNAVAILABLE
            //  4: MQTT_CONNECT_BAD_CREDENTIALS
            //  5: MQTT_CONNECT_UNAUTHORIZED
            retries++;
            delay(2000); 
        }
    }

    if (!_mqttClient.connected()) {
        LOG_ERROR("MQTT bağlantısı %d denemede başarısız oldu.", retries);
        return false;
    }
    return true;
}

int Communication_Driver::restartGPRS() {
    LOG_INFO("GPRS yeniden başlatılıyor...");
    if (_modem.isGprsConnected()) {
        _modem.gprsDisconnect();
        LOG_INFO("GPRS bağlantısı kesildi.");
    } else {
        LOG_INFO("GPRS zaten bağlı değil.");
    }
    delay(1000); 

    LOG_INFO("Yeni GPRS bağlantısı kuruluyor...");
    if (this->connectGPRS()) { // Sınıfın metodunu kullanarak GPRS'e bağlanmayı deneyin
        LOG_INFO("GPRS başarıyla yeniden bağlandı.");
        return 0; 
    } else {
        LOG_ERROR("GPRS yeniden bağlanma başarısız!");
        return 1; 
    }
}


String Communication_Driver::createCsvDataLine() {
    String csv_row = "";
    char buffer[20]; // For float to string conversion
    // 1. measurement_id
    csv_row += String(wakeCounter);
    csv_row += ",";
    // 2. measurement_time
    csv_row += _rtc.getTimestamp();
    csv_row += ",";
    // 3. soil_nitrogen
    dtostrf(_npkSensor.getAzot(), 4, 1, buffer); csv_row += buffer; csv_row += ",";
    // 4. soil_phosphorus
    dtostrf(_npkSensor.getFosfor(), 4, 1, buffer); csv_row += buffer; csv_row += ",";
    // 5. soil_potassium
    dtostrf(_npkSensor.getPotasyum(), 4, 1, buffer); csv_row += buffer; csv_row += ",";
    // 6. soil_humidity (NPK Nem)
    dtostrf(_npkSensor.getNem(), 4, 1, buffer); csv_row += buffer; csv_row += ",";
    // 7. soil_temperature (NPK Sicaklik)
    dtostrf(_npkSensor.getSicaklik(), 4, 1, buffer); csv_row += buffer; csv_row += ",";
    // 8. soil_electrical_conductivity (NPK EC)
    dtostrf(_npkSensor.getEC(), 4, 2, buffer); csv_row += buffer; csv_row += ",";
    // 9. soil_ph (NPK pH)
    dtostrf(_npkSensor.getPH(), 4, 2, buffer); csv_row += buffer; csv_row += ",";
    // soil_salinity
    dtostrf(_npkSensor.getSalinity(), 4, 2, buffer); csv_row += buffer; csv_row += ",";
    // soil_TDS
    dtostrf(_npkSensor.getTDS(), 4, 2, buffer); csv_row += buffer; csv_row += ",";
    // 10. weather_air_temperature (BME280 Temp)
    dtostrf(_bmeSensor.getTemperature(), 4, 1, buffer); csv_row += buffer; csv_row += ",";
    // 11. weather_air_humidity (BME280 Hum)
    dtostrf(_bmeSensor.getHumidity(), 4, 1, buffer); csv_row += buffer; csv_row += ",";
    // 12. weather_air_pressure (BME280 Press)
    dtostrf(_bmeSensor.getPressure(), 6, 2, buffer); csv_row += buffer; csv_row += ","; // Assuming pressure is in Pa, convert to hPa
    // 14. system_solar_panel_voltage (Placeholder)
    dtostrf(_sup_solar_external, 4, 2, buffer); csv_row += buffer; csv_row += ",";
    // 13. system_battery_voltage (_sup_bat_external)
    dtostrf(_sup_bat_external, 4, 2, buffer); csv_row += buffer; csv_row += ",";
    // 15. system_supply_voltage_1 (_sup_4v - modem battery)
    csv_row += String(_sup_4v); csv_row; // _sup_4v is already a char array (string)
    LOG_INFO("CSV Data Line: %s", csv_row.c_str());
    return csv_row;
}

String Communication_Driver::createJsonPayloadForAWS(const String& csv_content) {
    JsonDocument doc; 
    
        String versionId = readFile(SD, VERSION_ID_FILE);
        if (versionId.length() > 0) {
        versionId.trim();
        } else {
             writeFile(SD, VERSION_ID_FILE, 0);
             versionId = "0"; // Eğer dosya boşsa, versionId'yi 0 olarak ayarla
        }

    doc["hash_code"] = "a1b2c3d4e5f67890a1b2c3d4e5f67890a1b2c3d4e5f67890a1b2c3d4e5f67890";
    JsonObject header = doc["header"].to<JsonObject>();
    header["transmit_time"] = _rtc.getTimestamp();
    header["device_id"] = deviceId; 
    header["version_id"] = versionId; // Version ID'yi JSON'a ekle
    header["measurement_period"] = (unsigned long)(uykusuresi / 1000000ULL); // Saniye cinsinden
    header["transmit_period"] = (unsigned long)(uykusuresi / 1000000ULL) * MAX_WAKECOUNTER; // Saniye cinsinden
    header["measurement_count"] = MAX_WAKECOUNTER;
    header["status"] = "active";
    header["csv"] = csv_content; 

    String jsonBuffer;
    serializeJson(doc, jsonBuffer);

    LOG_INFO("AWS JSON Yükü Oluşturuldu. Uzunluk: %d", jsonBuffer.length());
    LOG_DEBUG("AWS JSON: %s", jsonBuffer.c_str());
    return jsonBuffer;
}

bool Communication_Driver::publishData(const char* payload) {
    if (!_mqttClient.connected()) {
        LOG_ERROR("MQTT bağlı değil, veri yayınlanamıyor.");
        return false;
    }

    LOG_INFO("MQTT Konusuna gönderiliyor: %s", AWS_IOT_PUBLISH_TOPIC);
    LOG_DEBUG("Payload: %s", payload); // payload artık bir parametre

    const int maxRetries = 5; // Orijinal publishData ile tutarlı
    for (int attempt = 1; attempt <= maxRetries; ++attempt) {
        if (_mqttClient.publish(AWS_IOT_PUBLISH_TOPIC, payload)) { // payload'ı doğrudan kullan
            LOG_INFO("Mesaj %d. denemede başarıyla yayınlandı.", attempt);
            return true;
        } else {
            LOG_WARN("MQTT mesaj yayınlama %d. denemede başarısız!", attempt);
            delay(500);
        }
    }

    LOG_ERROR("MQTT mesaj yayınlama %d denemeden sonra başarısız oldu.", maxRetries);
    return false;
}

void Communication_Driver::mqttLoop() {
    if (_mqttClient.connected()) {
        _mqttClient.loop();
    }
}

bool Communication_Driver::isMqttConnected() {
    return _mqttClient.connected();
}


void Communication_Driver::disconnect() {
    if (_mqttClient.connected()) {
        _mqttClient.disconnect();
        LOG_INFO("MQTT bağlantısı kapatıldı.");
    }
    if (_modem.isGprsConnected()) {
        _modem.gprsDisconnect();
        LOG_INFO("GPRS bağlantısı kapatıldı.");
    }
    
    LOG_INFO("Modem kapatılıyor...");
    if (_modem.poweroff()) { // SIM800 serisi için poweroff() 
        LOG_INFO("Modem başarıyla kapatıldı.");
    } else {
        LOG_WARN("Modem kapatılamadı veya zaten kapalıydı.");
    }
    delay(500); // Modeme kapanması için zaman verin
}

void Communication_Driver::updateModemBatteryStatus() {
    int voltageMV = _modem.getBattVoltage(); // Voltajı mV olarak döndürür
    if (voltageMV > 0) {
        float voltageV = voltageMV > 0 ? voltageMV / 1000.0 : 0.0;
        dtostrf(voltageV, 5, 2, _sup_4v); // SUP_4V'yi doldur
        LOG_INFO("Modem Batarya Voltajı: %sV", _sup_4v);
    } else {
        LOG_WARN("Modem batarya voltajı okunamadı.");
        strcpy(_sup_4v, "N/A");
    }
}

float Communication_Driver::readAndProcessBatteryVoltage() {
    // Son 5 ölçümün ortalamasını almak için döngü
    for (int i = 0; i < 5; i++) {
        int rawValue = analogRead(BAT_VOLTAGE_PIN);
        float voltage_adc = (rawValue / ADC_MAX_VALUE) * ADC_VREF;
        float current_battery_voltage = voltage_adc * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2;
        _batteryVoltageAvg.addValue(current_battery_voltage); // Değeri RunningAverage nesnesine ekle
        delay(10); // Ölçümler arasında kısa bir gecikme eklenebilir
    }
    _sup_bat_external = _batteryVoltageAvg.getAverage(); // Ortalama değeri al
    _sup_bat_external = round(_sup_bat_external * 100.0) / 100.0; // İki ondalık basamağa yuvarla
    
    LOG_INFO("=> Ortalama Pil Voltajı: %.2fV (%d ölçüm)", _sup_bat_external, _batteryVoltageAvg.getCount());
    _batteryVoltageAvg.clear(); // Bir sonraki ölçüm seti için ortalamayı temizle
    return _sup_bat_external;
}

float Communication_Driver::readAndProcessSolarVoltage() {
    // Son 5 ölçümün ortalamasını almak için döngü
    for (int i = 0; i < 5; i++) {
        int solar_rawValue = analogRead(SOLAR_VOLTAGE_PIN);
        float solar_voltage_adc = (solar_rawValue / ADC_MAX_VALUE) * ADC_VREF;
        float current_solar_voltage = solar_voltage_adc * (SOLAR_VOLTAGE_DIVIDER_R1 + SOLAR_VOLTAGE_DIVIDER_R2) / SOLAR_VOLTAGE_DIVIDER_R2;
        _solarVoltageAvg.addValue(current_solar_voltage); // Değeri RunningAverage nesnesine ekle
        delay(10); // Ölçümler arasında kısa bir gecikme eklenebilir
    }
    _sup_solar_external = _solarVoltageAvg.getAverage(); // Ortalama değeri al
    _sup_solar_external = round(_sup_solar_external * 100.0) / 100.0; // İki ondalık basamağa yuvarla

    LOG_INFO("=> Ortalama Solar Panel Voltajı: %.2fV (%d ölçüm)", _sup_solar_external, _solarVoltageAvg.getCount());
    _solarVoltageAvg.clear(); // Bir sonraki ölçüm seti için ortalamayı temizle
    return _sup_solar_external;
}
// Statik geri çağırma işlevi sarmalayıcısı
void Communication_Driver::staticMqttCallback(char* topic, byte* payload, unsigned int length) {
    // 1. Gelen veriyi null-terminated bir string'e çevir
    char msg[length + 1];
    memcpy(msg, payload, length);
    msg[length] = '\0';
    LOG_INFO("Gelen Mesaj [%s]: %s", topic, msg);

    // 2. JSON'u ayrıştır
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, msg);

    if (err) {
        LOG_ERROR("JSON ayrıştırılamadı! Hata: %s", err.c_str());
        return;
    }

    // 3. Cihaz ID'sini kontrol et
    const char* received_device_id = doc["header"]["device_id"];
    if (!received_device_id || deviceId.compareTo(received_device_id) != 0) {
        LOG_WARN("Mesaj bu cihaz için değil. Alınan ID: %s, Cihaz ID: %s",
                 received_device_id ? received_device_id : "YOK",
                 deviceId.c_str());
        return;
    }
    LOG_INFO("Mesaj bu cihaza ait.");

    // 4. OTA ve GPS isteklerini kontrol et
    if (doc.containsKey("header") && doc["header"].is<JsonObject>()) {
        JsonObject header = doc["header"];

        // OTA işlemini kontrol et
        if (header.containsKey("ota_request") && header["ota_request"].is<JsonObject>()) {
            JsonObject ota_request = header["ota_request"];

            // URL ve version_id varsa, işlem yapılır
            if (ota_request.containsKey("url") && ota_request.containsKey("version_id")) {
                const char* ota_url = ota_request["url"];
                const char* version_id = ota_request["version_id"];

                if (ota_url && version_id && strlen(ota_url) > 0 && strlen(version_id) > 0) {
                    LOG_INFO("OTA güncelleme mesajı alındı. URL: %s, Version: %s", ota_url, version_id);

                    if (writeFile(SD, VERSION_ID_FILE, version_id)) {
                        LOG_INFO("Yeni version_id (%s) SD karta kaydedildi.", version_id);
                    } else {
                        LOG_ERROR("Yeni version_id (%s) SD karta kaydedilemedi!", version_id);
                    }

                    if (_instance) {
                        _instance->performOTA(ota_url);
                    } else {
                        LOG_ERROR("Communication_Driver örneği bulunamadı, OTA başlatılamıyor.");
                    }
                } else {
                    LOG_INFO("OTA isteği var ancak url veya version_id eksik/boş.");
                }
            } else {
                LOG_INFO("OTA güncellemesi gerekmiyor (anahtarlar eksik veya boş).");
            }
        } else {
            LOG_INFO("OTA isteği bulunamadı veya geçersiz.");
        }

        // GPS isteğini kontrol et
        if (header.containsKey("gps_request")) {
            const char* gps_req = header["gps_request"];
            if (gps_req && strcmp(gps_req, "true") == 0) {
                LOG_INFO("GPS isteği alındı: true");
                
                if (_instance) {
                   _instance->disconnect();  // MQTT + GPRS bağlantısını kapat
               }
               delay(1000); // Donanıma stabilizasyon için zaman tanı

                
                if (_instance && _instance->enableGPS()) {
                    LOG_INFO("GPS modülü etkinleştirildi, konum alınıyor...");
                }
                if (_instance && _instance->readGPSWithRetry()) {
                    _instance->updateRtcWithGpsTime();
                    
                    StaticJsonDocument<256> gpsDoc;
                    gpsDoc["device_id"] = deviceId;
                    gpsDoc["Location"] = _instance->Location;  // "40.123456,29.123456" formatı
                    gpsDoc["gpsTime"] = _instance->gpsTime; // "2025-06-19 13:45:00" formatı

                    String gpsPayload;
                    serializeJson(gpsDoc, gpsPayload);
                    if (_instance && _instance->disableGPS()) {
                        LOG_INFO("GPS modülü devre dışı bırakıldı.");
                    }
                    if (_instance->connectMQTT()) {
                        _instance->publishData(gpsPayload.c_str());
                        LOG_INFO("GPS verisi MQTT ile sunucuya gönderildi.");
                    } 
                    else {
                         LOG_ERROR("MQTT bağlantısı kurulamadı, GPS verisi gönderilemedi.");
                    }
                } else {
                     LOG_WARN("GPS verisi alınamadı, gönderim yapılmadı.");
                }
            } else {
                LOG_INFO("GPS isteği false veya geçersiz, işlem yapılmadı.");
            }
        } else {
            LOG_INFO("GPS isteği bulunamadı.");
        }

    } else {
        LOG_WARN("Gelen JSON'da 'header' objesi bulunamadı.");
    }
}


void Communication_Driver::performOTA(const char* url) {
    LOG_INFO("OTA güncellemesi başlatılıyor. URL: %s", url);

    // --- 1. URL'yi Ayrıştır ---
    String urlStr = String(url);
    String host;
    String path;
    int port = 80;

    int protocolEnd = urlStr.indexOf("://");
    if (protocolEnd == -1) {
        LOG_ERROR("OTA: Geçersiz URL formatı.");
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    String protocol = urlStr.substring(0, protocolEnd);
    String remainingUrl = urlStr.substring(protocolEnd + 3);

    int pathStart = remainingUrl.indexOf('/');
    if (pathStart == -1) {
        host = remainingUrl;
        path = "/";
    } else {
        host = remainingUrl.substring(0, pathStart);
        path = remainingUrl.substring(pathStart);
    }
    
    // Protokole göre portu ve kullanılacak istemciyi belirle
    Client* netClient;
    if (protocol == "http") {
        netClient = &_gsmClient;
        LOG_INFO("OTA: HTTP kullanılacak.");
    } else {
        LOG_ERROR("OTA: Desteklenmeyen protokol: %s", protocol.c_str());
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    LOG_INFO("OTA: Host: %s, Port: %d, Path: %s", host.c_str(), port, path.c_str());

    // --- 2. Firmware'i SD Karta İndir ---
    LOG_INFO("OTA: Firmware indiriliyor -> %s", OTA_FIRMWARE_FILENAME);
    
    if (!_modem.isGprsConnected() && !connectGPRS()) {
        LOG_ERROR("OTA için GPRS bağlantısı kurulamadı.");
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }
    
    HttpClient client(*netClient, host.c_str(), port);
    client.beginRequest();
    int err = client.get(path.c_str());
    client.endRequest();

    if (err != 0) {
        LOG_ERROR("OTA: HttpClient GET isteği başarısız. Hata: %d", err);
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    int httpCode = client.responseStatusCode();
    LOG_INFO("OTA: HTTP durum kodu: %d", httpCode);

    if (httpCode != 200) {
        LOG_ERROR("OTA: HTTP hata kodu: %d", httpCode);
        String body = client.responseBody();
        LOG_ERROR("OTA: HTTP Yanıtı: %s", body.c_str());
        return;
    }

    int contentLength = client.contentLength();
    if (contentLength <= 0) {
        LOG_ERROR("OTA: Geçersiz içerik uzunluğu.");
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    LOG_INFO("OTA: Firmware boyutu: %d bayt", contentLength);
    
    // SD Karta yazma işlemi
    if (SD.exists(OTA_FIRMWARE_FILENAME)) {
        SD.remove(OTA_FIRMWARE_FILENAME);
        LOG_INFO("OTA: Mevcut firmware dosyası silindi: %s", OTA_FIRMWARE_FILENAME);
    }

    File updateFile = SD.open(OTA_FIRMWARE_FILENAME, FILE_WRITE);
    if (!updateFile) {
        LOG_ERROR("OTA: SD kartta dosya oluşturulamadı: %s", OTA_FIRMWARE_FILENAME);
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    uint8_t buff[256] = { 0 };
    int written = 0;
    int progressPercent = -1;
    unsigned long timeout = millis();

    LOG_INFO("OTA: İndirme başlıyor...");
    while (written < contentLength && (millis() - timeout < 300000L)) { // 5 dakika zaman aşımı
        int len = client.read(buff, sizeof(buff));
        if (len > 0) {
            updateFile.write(buff, len);
            written += len;
            timeout = millis();

            int currentProgress = (written * 100) / contentLength;
            if (currentProgress > progressPercent) {
                progressPercent = currentProgress;
                LOG_INFO("OTA: İndiriliyor: %d%% (%d / %d)", progressPercent, written, contentLength);
            }
        } else if (len == 0) {
            delay(50);
        } else {
            LOG_ERROR("OTA: İndirme sırasında okuma hatası!");
            writeFile(SD, VERSION_ID_FILE, 0);
            break;
        }
    }
    
    updateFile.close();
    client.stop();

    if (written != contentLength) {
        LOG_ERROR("OTA: İndirme tamamlanamadı. İndirilen: %d, Beklenen: %d", written, contentLength);
        SD.remove(OTA_FIRMWARE_FILENAME); // Tamamlanmamış dosyayı sil
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }
    
    LOG_INFO("OTA: Firmware başarıyla SD karta indirildi.");

    // --- 3. SD Karttan Güncellemeyi Uygula ---
    LOG_INFO("OTA: SD karttan güncelleme uygulanıyor...");
    File file = SD.open(OTA_FIRMWARE_FILENAME, FILE_READ);
    if (!file) {
        LOG_ERROR("OTA: İndirilen firmware dosyası açılamadı.");
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    size_t fileSize = file.size();
    if (fileSize == 0) {
        LOG_ERROR("OTA: Firmware dosyası boş.");
        file.close();
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }
    
    if (!Update.begin(fileSize)) {
        LOG_ERROR("OTA: Güncelleme başlatılamadı. Yetersiz alan olabilir. Hata: %s", Update.errorString());
        file.close();
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    size_t writtenToFlash = Update.writeStream(file);
    file.close();

    if (writtenToFlash != fileSize) {
        LOG_ERROR("OTA: Flash'a yazma hatası. Yazılan: %d, Beklenen: %d", writtenToFlash, fileSize);
        LOG_ERROR("OTA: Hata Detayı: %s", Update.errorString());
        writeFile(SD, VERSION_ID_FILE, 0);
        return;
    }

    if (Update.end(true)) {
        LOG_INFO("OTA: Güncelleme başarıyla tamamlandı!");
        LOG_INFO("OTA: Cihaz yeniden başlatılıyor...");
        delay(1000);
        ESP.restart();
    } else {
        LOG_ERROR("OTA: Güncelleme hatası! Hata: %s", Update.errorString());
        writeFile(SD, VERSION_ID_FILE, 0);
    }
}

// Yeni eklenen fonksiyon: GPS verisi ile RTC'yi günceller
void Communication_Driver::updateRtcWithGpsTime() {
    if (_gps_fix_available && _gps_year > 2023) { // GPS'ten geçerli bir zaman alındıysa
        LOG_INFO("RTC, GPS zamanı ile güncelleniyor: %s", gpsTime);
        _rtc.setDateTime(_gps_year, _gps_month, _gps_day, _gps_hour, _gps_minute, _gps_second);
    } else {
        LOG_WARN("RTC güncellemesi için geçerli GPS zamanı yok.");
    }
}