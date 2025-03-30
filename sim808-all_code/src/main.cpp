#include <Arduino.h>
#include "ModuleSerialGsm.h" 
#include "ModuleSerialCore.h"
#include "ModuleSerialGprs.h"
#include "LoggerModule.h"
#include "Config.h"
#include "NpkSensor.h"
#include "esp_sleep.h"
#include "esp_system.h"
#include <ArduinoJson.h>

// 🔹 **Seri haberleşme için donanım UART tanımlama**
HardwareSerial serialPort(2);  
HardwareSerial modbusSerial(1);

// 🔹 **Modül nesneleri**
ModuleSerialCore core(serialPort, UART_RX_PIN, UART_TX_PIN);  
ModuleSerialGprs gprs(&core);  
ModuleSerialGsm gsm(&core);  

// 🔹 **NPK Sensör Nesnesi**
NpkSensor npkSensor(modbusSerial, 1);  // Modbus adresi 1 olarak ayarlandı

// 🔹 **Sunucu Bilgileri**
const char* serverUrl = "http://54.209.113.164/iot/";
const char* apiKey = "secret-api-key-trying-tarim-iot-app";

// 🔹 **Zamanlama Değişkenleri**
unsigned long previousMillis = 0;
const long interval = 5000;  
#define uykusuresi 60000000  // 2 dakika (mikrosaniye cinsinden)

// 🔹 **Cihaz ID'si**
const char* DEVICE_ID = "002";

// 🔹 **Batarya ve Sensör Değişkenleri**
char voltageStr[6];  // 5 karakter + null karakter
char dateTimeStr[64] = {0};

// 🔹 **Fonksiyon Prototipleri**
void sendIoTData();
String createJsonPayload();

void setup() {
    Serial.begin(115200);
    LoggerInit();  // Log sistemini başlat
    
    // 🔹 **NPK Sensörü Başlat**
    npkSensor.begin();

    // 🔹 **GPRS Modülü Başlat**
    Serial.println("GPRS modülü başlatılıyor...");
    while (core.begin(GPS_GSM_BAUDRATE) == MODULE_FAIL) {
        Serial.println("GPRS başlatılamadı! Tekrar deneniyor...");
        delay(2000);
    }
    Serial.println("✅ GPRS başlatıldı!");

    // 🔹 **GPRS Bağlantısını Aç**
    while (gprs.enable(APN, APN_USERNAME, APN_PASSWORD) != GPRS_ENABLED) {
        Serial.println("GPRS bağlantısı başarısız!");
        delay(2000);
    }
    Serial.println("GPRS bağlantısı başarılı!");
    
} 

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // 🔹 **NPK Sensörünü Oku**
        bool npk_success = false;
        int retry_count = 0;
        const int max_retries = 10;

        while (retry_count < max_retries) {
            if (npkSensor.readData()) {
                npk_success = true;
                break;
            }
            retry_count++;
            Serial.println(" NPK sensörü okunuyor...");
            delay(1000);
        }

        if (!npk_success) {
            Serial.println("NPK sensörüne erişilemedi!");
        }

        // 🔹 **Sensör Verilerini Yazdır**
        Serial.println("----- Gerçek Sensör Verileri -----");
        Serial.printf("Nem: %.1f %%\n", npkSensor.getNem());
        Serial.printf("Sıcaklık: %.1f °C\n", npkSensor.getSicaklik());
        Serial.printf("İletkenlik: %.0f uS/cm\n", npkSensor.getEC());
        Serial.printf("pH: %.1f\n", npkSensor.getPH());
        Serial.printf("Azot: %.0f mg/kg\n", npkSensor.getAzot());
        Serial.printf("Fosfor: %.0f mg/kg\n", npkSensor.getFosfor());
        Serial.printf("Potasyum: %.0f mg/kg\n", npkSensor.getPotasyum());

        // 🔹 **Batarya Bilgilerini Al**
        ModuleSerialGsm::BatteryStatus battery = gsm.currentBatteryStatus();
        float voltageV = battery.voltage / 1000.0;
        dtostrf(voltageV, 5, 2, voltageStr);

        Serial.printf("Batarya Kapasitesi: %d%%\n", battery.capacity);
        Serial.printf("Batarya Voltajı: %.2fV\n", voltageV);

        // 🔹 **IoT Sunucusuna Veri Gönder**
        sendIoTData();
        
        delay(2000);

        // 🔹 **ESP32'yi Uyku Moduna Al**
        Serial.println("ESP32 derin uyku moduna geçiyor...");
        Serial.flush();
        esp_sleep_enable_timer_wakeup(uykusuresi);
        esp_deep_sleep_start();
    }
}

// 📌 **JSON Verisini Hazırla**
String createJsonPayload() {
    JsonDocument doc;
    
    doc["DEVICE_ID"] = DEVICE_ID;
    doc["AZOT"] = npkSensor.getAzot();
    doc["BATARYA_VOLTAJI"] = voltageStr;
    doc["EC"] = npkSensor.getEC();
    doc["FOSFOR"] = npkSensor.getFosfor();
    doc["HAVA_BASINC"] = 0;  // Varsayılan değer
    doc["HAVA_NEM"] = 0;     // Varsayılan değer
    doc["HAVA_SICAKLIK"] = 0;// Varsayılan değer
    doc["NEM"] = npkSensor.getNem();
    doc["NPK_ERROR_CODE"] = npkSensor.getErrorCode();
    doc["PH"] = npkSensor.getPH();
    doc["POTASYUM"] = npkSensor.getPotasyum();
    doc["SICAKLIK"] = npkSensor.getSicaklik();

    String jsonBuffer;
    serializeJson(doc, jsonBuffer);
    return jsonBuffer;
}

// 📌 **HTTP POST ile IoT Sunucusuna Veri Gönder**
void sendIoTData() {
    if (core.isReady() == MODULE_FAIL) {
        Serial.println("GPRS modülü çalışmıyor! Yeniden başlatılıyor...");
        core.begin(GPS_GSM_BAUDRATE);
        delay(2000);
        gprs.enable(APN, APN_USERNAME, APN_PASSWORD);
    }

    for (int i = 0; i < 5; i++) {
        Serial.println("Sunucuya veri gönderiliyor...");
    
        // HTTP bağlantısını aç
        gprs.openHttpConnection();
    
        // JSON verisini hazırla
        String jsonPayload = createJsonPayload();
    
        // HTTP parametrelerini ayarla
        core.writeCommand("AT+HTTPPARA=\"CID\",1", "OK", 2000);
        core.writeCommand(("AT+HTTPPARA=\"URL\",\"" + String(serverUrl) + "\"").c_str(), "OK", 2000);
        core.writeCommand("AT+HTTPPARA=\"CONTENT\",\"application/json\"", "OK", 2000);
        core.writeCommand(("AT+HTTPPARA=\"USERDATA\",\"x-api-key: " + String(apiKey) + "\"").c_str(), "OK", 2000);
         
        // JSON verisini gönder
        char command[50];
        sprintf(command, "AT+HTTPDATA=%d,5000", jsonPayload.length());
        core.writeCommand(command, "DOWNLOAD", 2000);
        core.writeCommand(jsonPayload.c_str(), "OK", 5000);
         
        // HTTP POST isteğini yap
        core.writeCommand("AT+HTTPACTION=1", "+HTTPACTION:", 5000);
       
        // Sunucu yanıtını oku
        char response[50];
        gprs.readHttpResponse(50, response, sizeof(response));
        Serial.printf("Sunucu Yanıtı: %s\n", response);
        
        // HTTP bağlantısını kapat
        gprs.closeHttpConnection();

        delay(1000);
    }
}  
