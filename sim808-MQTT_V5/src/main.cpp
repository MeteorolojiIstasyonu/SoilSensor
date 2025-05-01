#include <Arduino.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h> // Seri haberleşme için
#include "LoggerModule.h"
#include "Config.h"
#include "NpkSensor.h"
#include "BME280Sensor.h" // BME280 sensörü için
#include "secrets.h" // Sertifikalar ve gizli bilgiler için
#include "esp_sleep.h"
#include "esp_system.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
// TinyGSM Kütüphaneleri
#define TINY_GSM_MODEM_SIM800 // Kullandığınız modeme göre ayarlayın (SIM808 için SIM800 genellikle çalışır)
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <SSLClient.h> // Güvenli bağlantı için

float SUP_BAT = 0.0; // Pil voltajı
float voltage_adc = 0.0; // ADC pinindeki voltaj
char SUP_4V[6];  // Batarya voltajı
int rawValue = 0; // ADC pininden okunan ham değer
// 🔹 **Zamanlama Değişkenleri**
unsigned long previousMillis = 0;


static const char* aws_root_ca_pem PROGMEM = R"EOF(
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

static const char* aws_certificate_pem PROGMEM = R"KEY(
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

static const char* aws_private_pem PROGMEM = R"KEY(
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

//     **SIM808 için**
HardwareSerial serialPort(2);  
TinyGsm modem(serialPort);
TinyGsmClient gsmClient(modem); // TinyGSM client oluştur
SSLClient secureClient(&gsmClient); // TinyGSM client'ını kullan
PubSubClient mqttClient(secureClient); // Güvenli client'ı kullan
// 🔹 **Sensör Nesnesi**
HardwareSerial modbusSerial(1); // NPK için
NpkSensor npkSensor(modbusSerial, 1);  // Modbus adresi 1 olarak ayarlandı
BME280Sensor bmeSensor; // BME280 sensörü için nesne oluştur
// 🔹 **Fonksiyonlar **
void connectGPRS();
void connectMQTT();
void publishData();
String createJsonPayload();
void setupModem();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {

    Serial.begin(SERIAL_BAUD_RATE); // Debug için
    serialPort.begin(9600, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN); // TinyGSM için seri portu başlat
    pinMode(PWR, OUTPUT); delay(500); // PWR pinini çıkış olarak ayarla
    LoggerInit();  // Log sistemini başlat
    LOG_INFO("Sistem Başlatılıyor...");
    
     // BME280 sensörünü başlat
     if (!bmeSensor.begin()) {
        Serial.println("BME280 sensörü bulunamadı!");
    } else {
        Serial.println("BME280 Başarıyla Başlatıldı!");
    }
    // 🔹 **NPK Sensörü Başlat**
    npkSensor.begin();
    LOG_INFO("NPK Sensörü başlatıldı.");

    LOG_INFO("Kurulum tamamlandı.");
}

void loop() {
    delay(1000); // Başlangıçta 1 saniye bekle
    // Pil Voltajını Hesapla (Config.h'daki değerleri kullanarak)
    rawValue = analogRead(VOLTAGE_PIN); // ADC pininden ham değeri oku
    voltage_adc = (rawValue / ADC_MAX_VALUE) * ADC_VREF; // Config.h'daki VREF ve ADC_MAX_VALUE kullanıldı
    SUP_BAT = voltage_adc * (VOLTAGE_DIVIDER_R1 + VOLTAGE_DIVIDER_R2) / VOLTAGE_DIVIDER_R2; // Config.h'daki R1 ve R2 kullanıldı
    SUP_BAT = round(SUP_BAT * 100.0) / 100.0; // 2 ondalık basamağa yuvarla
    LOG_INFO(" => ADC Voltajı (Okunan Değer): %.2fV", voltage_adc);
    LOG_INFO(" => Hesaplanan Pil Voltajı: %.2fV", SUP_BAT);

    // MQTT bağlantısını ve GPRS'i canlı tut
    connectMQTT(); // MQTT'yi bağla
    
    // MQTT arka plan işlemlerini yürüt (PING vb.)
    if (mqttClient.connected()) {
        mqttClient.loop();
    }

    // Belirli aralıklarla veri oku ve gönder
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= LOOP_INTERVAL_MS) {
        previousMillis = currentMillis;

        LOG_INFO("Sensör verileri okunuyor ve gönderiliyor...");
        bool npk_success = false;
        bool bme_success = false;
        int retry_count = 0;
        const int max_retries = 5; // Deneme sayısını azaltabiliriz

        // 🔹 **BME280 Sensör Verilerini Oku**
        while (retry_count < max_retries) {
            if (bmeSensor.begin()) { // BME280 verilerini oku
                bmeSensor.readData();
                bme_success = true;
                LOG_INFO("BME280 verisi başarıyla okundu.");
                break;
            }
            retry_count++;
            LOG_WARN("BME280 okuma denemesi %d başarısız.", retry_count);
            delay(1000);
        }
        if (!bme_success) {
            LOG_ERROR("BME280 sensörüne %d denemede erişilemedi!", max_retries);
            retry_count = 0; // BME280 başarısızsa NPK'yi okumaya geçelim
        }
        
        // 🔹 **NPK Sensör Verilerini Oku**
        while (retry_count < max_retries) {
            if (npkSensor.readData()) {
                npk_success = true;
                LOG_INFO("NPK verisi başarıyla okundu.");
                retry_count = 0; // NPK okuma başarılıysa döngüyü kır
                break;
            }
            retry_count++;
            npkSensor.begin(); // NPK sensörünü başlat
            LOG_WARN("NPK okuma denemesi %d başarısız.", retry_count);
            delay(1000);
        }
        if (!npk_success) {
            LOG_ERROR("NPK sensörüne %d denemede erişilemedi!", max_retries);
            retry_count = 0;
        }

        // 🔹 **Batarya Bilgilerini Al (TinyGSM ile)**
        int voltageMV = modem.getBattVoltage();
        float voltageV = voltageMV > 0 ? voltageMV / 1000.0 : 0.0;
        dtostrf(voltageV, 5, 2, SUP_4V); // SUP_4V'yi doldur
        LOG_INFO("Batarya Kapasitesi:Voltaj: %.2fV", voltageV);

        // 🔹 **Veriyi MQTT ile Gönder**
        if (mqttClient.connected()) {
             publishData(); // JSON oluşturup gönderecek
        } else {
            LOG_ERROR("MQTT bağlantısı yok, veri gönderilemedi!");
        }

        delay(2000); // Gönderim sonrası kısa bir bekleme

        // 🔹 **ESP32'yi Uyku Moduna Al**
        mqttClient.disconnect(); // MQTT bağlantısını kapat
        modem.gprsDisconnect(); // GPRS bağlantısını kapat
        modem.poweroff(); // Modemi kapat
        delay(1000); // Modem kapanması için bekle

        LOG_INFO("ESP32 %lu mikrosaniye derin uyku moduna geçiyor...", (unsigned long)uykusuresi);
        Serial.flush(); // Seri port buffer'ını boşalt
        esp_sleep_enable_timer_wakeup(uykusuresi);
        esp_deep_sleep_start(); // Uykuya dal
    }
}

// 📌 **Modem Ayarlarını Yap**
void setupModem() {
    LOG_INFO("Modem başlatılıyor...");
    // modem.restart(); // Modemi yeniden başlatmak gerekebilir
    digitalWrite(PWR, LOW); // SİM808'i AÇ
    delay(1000); // 2 saniye bekle
    digitalWrite(PWR, HIGH); 
    delay(1000);
    // Modemin yanıt vermesini bekle
    int retry = 0;
    while (!modem.testAT(1000U) && retry < 5) {
        LOG_WARN("AT komutuna yanıt yok, tekrar deneniyor... (%d)", retry + 1);
        digitalWrite(PWR, LOW); // SİM808'i AÇ
        delay(1000); // 2 saniye bekle
        digitalWrite(PWR, HIGH); 
        delay(1000);
        retry++;
    }
    if (retry >= 10) {
        LOG_ERROR("Modem yanıt vermiyor!");
        // Burada sistemi yeniden başlatma gibi bir aksiyon alınabilir
        ESP.restart();
    }
    LOG_INFO("Modem AT yanıtı alındı.");

    // SIM kart PIN kontrolü
    if (strlen(SIM_CARD_PIN) > 0) {
        LOG_INFO("SIM PIN giriliyor...");
        if (!modem.simUnlock(SIM_CARD_PIN)) {
            LOG_ERROR("SIM PIN kilidi açılamadı!");
            // Hata yönetimi
        }
    }

    // SIM kart durumunu kontrol et
    SimStatus simStatus = modem.getSimStatus();
     while (simStatus != SIM_READY) {
        LOG_WARN("SIM Kart hazır değil: %d, bekleniyor...", simStatus);
        delay(1000);
        simStatus = modem.getSimStatus();
    }
    LOG_INFO("SIM Kart hazır.");
    LOG_INFO("Modem ayarları tamamlandı.");
}

// 📌 **GPRS Bağlantısını Kur**
void connectGPRS() {
    if (modem.isGprsConnected()) {
        LOG_INFO("GPRS zaten bağlı.");
        return;
    }
    LOG_INFO("GPRS bağlantısı kuruluyor...");
    LOG_INFO("Ağa bağlanılıyor...");
    if (!modem.waitForNetwork(60000L)) { // 1 dakika bekle
        LOG_ERROR("Ağ bağlantısı zaman aşımına uğradı!");
        setupModem(); // Modemi yeniden başlatmayı dene
        return;
    }
    LOG_INFO("Ağ bağlantısı OK.");
    LOG_INFO("Sinyal kalitesi: %d", modem.getSignalQuality());

    LOG_INFO("GPRS'e bağlanılıyor...");
    if (!modem.gprsConnect(APN, APN_USERNAME, APN_PASSWORD)) {
        LOG_ERROR("GPRS bağlantısı başarısız!");
        delay(10000);
        return;
    }
    LOG_INFO("GPRS Bağlantısı OK. IP: %s", modem.getLocalIP().c_str());
}

// 📌 **MQTT Bağlantısını Kur**
void connectMQTT() {
    LOG_INFO("Modem başlatılıyor...");
    setupModem(); // Modem ayarlarını yap
    connectGPRS(); // 🔹 **GPRS Bağlantısını Kur**
    
    if (!modem.isNetworkConnected()) {
        LOG_WARN("Ağ bağlantısı koptu, tekrar bağlanılıyor...");
        setupModem(); // Modem ayarlarını yap
        connectGPRS(); // GPRS'i tekrar bağla
    }


    LOG_INFO("SSL İstemcisi ayarlanıyor...");
    secureClient.setCACert(aws_root_ca_pem);
    secureClient.setCertificate(aws_certificate_pem);
    secureClient.setPrivateKey(aws_private_pem);
    // 🔹 **MQTT Sunucusunu ve Portunu Ayarla**
    mqttClient.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
    LOG_INFO("MQTT sunucusuna bağlanılıyor: %s:%d", AWS_IOT_ENDPOINT, AWS_IOT_PORT);
    mqttClient.setCallback(callback); // Gelen mesajları dinle

     // Bağlanmayı dene 
     int retries = 0;
     while (!mqttClient.connected() && retries < 5) {
          LOG_INFO("MQTT Bağlantı denemesi %d...", retries + 1);
 
         // Client ID ile bağlan
         if (mqttClient.connect(MQTT_CLIENT_ID)) {
            LOG_INFO("MQTT Bağlandı!");
            mqttClient.subscribe(AWS_IOT_SUBSCRIBE_TOPIC); // Abone olun
            if (mqttClient.subscribe(AWS_IOT_SUBSCRIBE_TOPIC)) {
                LOG_INFO("MQTT konusuna abone olundu: %s", AWS_IOT_SUBSCRIBE_TOPIC);
            } else {
                LOG_ERROR("MQTT konusuna abone olma başarısız!");
            }
            return; // Başarılı bağlantı, fonksiyondan çık

        } else {
             LOG_ERROR("MQTT bağlantı hatası, rc=%d", mqttClient.state());
             // Hata kodlarını PubSubClient dokümantasyonunda bulabilirsiniz.
             // -4: MQTT_CONNECTION_TIMEOUT
             // -3: MQTT_CONNECTION_LOST
             // -2: MQTT_CONNECT_FAILED
             // -1: MQTT_DISCONNECTED
             // 1: MQTT_CONNECT_BAD_PROTOCOL
             // 2: MQTT_CONNECT_BAD_CLIENT_ID
             // 3: MQTT_CONNECT_UNAVAILABLE
             // 4: MQTT_CONNECT_BAD_CREDENTIALS
             // 5: MQTT_CONNECT_UNAUTHORIZED
             LOG_ERROR(" Tekrar denemek için 2 saniye bekleniyor...");
             retries++;
             delay(2000);
         }
     }
      if (!mqttClient.connected()){
          LOG_ERROR("MQTT bağlantısı %d denemede başarısız oldu.", retries);
      }
    
}


// 📌 **JSON Verisini Hazırla**
String createJsonPayload() {
    JsonDocument doc; // ArduinoJson v kullanılıyor
    doc["DEVICE_ID"] = DEVICE_ID;
    doc["SUP_4V"] = SUP_4V; // String olarak gönderiyoruz
    doc["SUP_BAT"] = SUP_BAT; // Pil voltajı
    doc["SICAKLIK"] = npkSensor.getSicaklik();
    doc["NEM"] = npkSensor.getNem();
    doc["PH"] = npkSensor.getPH();
    doc["EC"] = npkSensor.getEC();
    doc["AZOT"] = npkSensor.getAzot();
    doc["FOSFOR"] = npkSensor.getFosfor();
    doc["POTASYUM"] = npkSensor.getPotasyum();
    doc["HAVA_SICAKLIK"] = bmeSensor.getTemperature(); // BME280'dan okunan sıcaklık
    doc["HAVA_NEM"] = bmeSensor.getHumidity(); // BME280'dan okunan nem
    doc["HAVA_BASINC"] = bmeSensor.getPressure(); // BME280'dan okunan basınç
    doc["NPK_ERROR_CODE"] = npkSensor.getErrorCode(); // NPK okuma hata kodu
    String jsonBuffer;
    serializeJson(doc, jsonBuffer);
    LOG_DEBUG("Oluşturulan JSON: %s", jsonBuffer.c_str());
    return jsonBuffer;
}

// 📌 **MQTT ile Veri Gönder**
void publishData() {
    String payload = createJsonPayload();

    LOG_INFO("MQTT Konusuna gönderiliyor: %s", AWS_IOT_PUBLISH_TOPIC);
    LOG_INFO("Payload: %s", payload.c_str());

    // MQTT'ye yayınla
    if (mqttClient.publish(AWS_IOT_PUBLISH_TOPIC, payload.c_str())) {
        LOG_INFO("Mesaj başarıyla yayınlandı.");
    } else {
        LOG_ERROR("MQTT mesaj yayınlama başarısız!");
    }
}

//Gelen MQTT Mesajları için Callback Fonksiyonu
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mesaj geldi [");
  Serial.print(topic);
  Serial.print("] ");
  JsonDocument doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
  // Gelen mesaja göre işlem yapabilirsiniz
}
