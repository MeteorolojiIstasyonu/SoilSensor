#include <Arduino.h>
#include <ArduinoJson.h> // May not be needed directly if all JSON is in driver
#include <inttypes.h>    // For PRIu64 definition
#include <HardwareSerial.h> // For serial communication
#include "LoggerModule.h"
#include "Config.h"
#include "NpkSensor.h"
#include "BME280Sensor.h"
#include "secrets.h"      // For secrets (though many are now used by driver)
#include "esp_sleep.h"
#include "esp_system.h"
#include <Wire.h>
#include <Adafruit_Sensor.h> // For BME280
#include "Communication_Driver.h" // Include the new communication driver

unsigned long previousMillis = 0; 


HardwareSerial modemSerial(2); // Modem için
HardwareSerial modbusSerial(1); // NPK için
NpkSensor npkSensor(modbusSerial, 1);  // Modbus adresi 1 olarak ayarlandı
BME280Sensor bmeSensor; // BME280 sensörü için nesne oluşturuldu

Communication_Driver commDriver(modemSerial, npkSensor, bmeSensor);

void setup() {
    Serial.begin(SERIAL_BAUD_RATE); // Debug serial port
    delay(100); // Wait for serial to initialize
   
    modemSerial.begin(9600, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);    
    pinMode(PWR_sim808, OUTPUT); 
    pinMode(PWRBME, OUTPUT); // PWR pinini çıkış olarak ayarla
    pinMode(PWRNPK, OUTPUT); // PWR pinini çıkış olarak ayarla
    digitalWrite(PWRBME, HIGH); // BME280 ve NPK sensörünü aç
    digitalWrite(PWRNPK, HIGH); // BME280 ve NPK sensörünü aç
    LoggerInit();
    LOG_INFO("Sistem Başlatılıyor...");
    delay(1000); // Wait for sensors to power up

    // BME280 sensörünü başlat
    if (!bmeSensor.begin()) {
        LOG_ERROR("BME280 sensörü bulunamadı!");
    } else {
        LOG_INFO("BME280 Başarıyla Başlatıldı!");
    }

    npkSensor.begin(); // This also sets up DE_RE pin
    LOG_INFO("NPK Sensörü başlatıldı.");    
    LOG_INFO("Kurulum tamamlandı.");
}

void loop() {
    commDriver.setupModem(); // Modemi kur
    commDriver.enableGPS(); // GPS'i etkinleştir
    commDriver.updateModemBatteryStatus();//bataryayı güncelle
    commDriver.readAndProcessBatteryVoltage(); // Harici pil voltajını oku
   
    if (!commDriver.isMqttConnected()) {
        LOG_INFO("MQTT bağlantısı yok, bağlantı kuruluyor...");
        if (!commDriver.connectMQTT()) { // Bu, modemi, GPRS'i ve ardından MQTT'yi bağlamayı dener
            LOG_ERROR("MQTT bağlantısı kurulamadı. Bir sonraki döngüde tekrar denenecek veya uykuya geçilecek.");
        }
    }

    // MQTT arka plan işlemlerini yürüt
    if (commDriver.isMqttConnected()) {
        commDriver.mqttLoop();
    }

    // Timed data reading and sending interval
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= LOOP_INTERVAL_MS) {
        previousMillis = currentMillis;

        LOG_INFO("Sensör verileri okunuyor ve gönderilecek...");
        // Güç AÇIK sensörleri (açık olduklarından emin olun)        
        digitalWrite(PWRBME, HIGH);
        digitalWrite(PWRNPK, HIGH);
        delay(500); // Short delay for sensors to stabilize after power on
        
        bmeSensor.readBME280WithRetry(); // BME280 sensör verilerini oku
        npkSensor.readNPKWithRetry();//  **NPK Sensör Verilerini Oku**
        commDriver.readGPSWithRetry(); // GPS konumunu al
        commDriver.disableGPS(); // GPS'i devre dışı bırak 
        
        // **Veriyi MQTT ile Gönder**
        if (commDriver.isMqttConnected()) {
            // publishData çağrıldığında, createJsonPayload içindeki _gps_lat vb. en son alınan değerleri kullanacaktır.
            if (commDriver.publishData()) {
                LOG_INFO("Veri başarıyla MQTT'ye gönderildi.");
            } else {
                LOG_ERROR("MQTT veri gönderimi başarısız.");
            }
        } else {
            // ... (MQTT bağlı değilse yapılacaklar)
        }
        delay(2000); // Veri göndermeye çalıştıktan sonra kısa gecikme
        // Derin Uyku
        LOG_INFO("Derin uyku moduna hazırlanılıyor...");
        // Power OFF sensors
        digitalWrite(PWRBME, LOW);
        digitalWrite(PWRNPK, LOW);
        LOG_INFO("BME280 ve NPK sensörleri kapatıldı.");
        // İletişimi kesin ve modemi kapatın
        commDriver.disconnect(); 
        previousMillis = 0;
        LOG_INFO("ESP32 %" PRIu64 " mikrosaniye derin uyku moduna geçiyor...", (uint64_t)uykusuresi);
        Serial.flush(); // Flush serial buffer before sleep
        esp_sleep_enable_timer_wakeup(uykusuresi);
        esp_deep_sleep_start(); // Enter deep sleep
    }
}
