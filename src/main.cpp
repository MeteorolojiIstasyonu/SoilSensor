#include <Arduino.h>
#include <WiFi.h>
#include <Update.h>
#include <ArduinoJson.h> 
#include <inttypes.h>    
#include <HardwareSerial.h> 
#include "LoggerModule.h"
#include "Config.h"
#include "NpkSensor.h"
#include "BME280Sensor.h"
#include "secrets.h"      
#include "esp_sleep.h"
#include "esp_system.h"
#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include "Communication_Driver.h" 
#include "sd_card.h"
#include "RTC_1302.h" 

String deviceId;

RTC_Module rtc(RTC_IO_PIN, RTC_SCLK_PIN, RTC_CE_PIN);

HardwareSerial modemSerial(2); 
HardwareSerial modbusSerial(1); 
NpkSensor npkSensor(modbusSerial, 1);  
BME280Sensor bmeSensor; 

Communication_Driver commDriver(modemSerial, npkSensor, bmeSensor, rtc);

RTC_DATA_ATTR int wakeCounter = 0;

const char* csvHeader = "measurement_id,measurement_time,soil_nitrogen,soil_phosphorus,soil_potassium,soil_humidity,soil_temperature,soil_electrical_conductivity,soil_ph,soil_salinity,soil_TDS,weather_air_temperature,weather_air_humidity,weather_air_pressure,system_solar_panel_voltage,system_battery_voltage,system_supply_4V\n";

void powerUpSensors() {
    digitalWrite(PWRBME, HIGH);
    digitalWrite(PWRNPK, HIGH);
    commDriver.pwrmodem();
    LOG_INFO("BME280 ve NPK sensörleri açıldı.");
}

void powerDownSensors() {
    digitalWrite(PWRBME, LOW);
    digitalWrite(PWRNPK, LOW);
    LOG_INFO("BME280 ve NPK sensörleri kapatıldı.");
}

void goToDeepSleep() {
    powerDownSensors();
    commDriver.disconnect();
    LOG_INFO("ESP32 %" PRIu64 " mikrosaniye derin uyku moduna geçiyor...", (uint64_t)uykusuresi);
    Serial.flush();
    esp_sleep_enable_timer_wakeup(uykusuresi);
    esp_deep_sleep_start();
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    LoggerInit();
    WiFi.mode(WIFI_STA);
    deviceId = WiFi.macAddress();
    deviceId.replace(":", "");
    LOG_INFO("Cihaz ID (MAC Adresi): %s", deviceId.c_str());

    pinMode(PWR_sim808, OUTPUT);
    pinMode(PWRBME, OUTPUT);
    pinMode(PWRNPK, OUTPUT);
    digitalWrite(PWR_sim808, HIGH);
    modemSerial.begin(9600, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);    
    powerUpSensors();
    delay(13000);
    rtc.begin(); 
    rtc.getTimestamp();
   
    LOG_INFO("Sistem Başlatılıyor...");

    if (initSDCard()) {
        if (ensureDataFileWithHeader(SD, SD_DATA_FILE, csvHeader)) {
            LOG_INFO("Veri dosyası (%s) başlığıyla birlikte hazır.", SD_DATA_FILE);
        } else {
            LOG_ERROR("Veri dosyası (%s) için başlık oluşturulamadı/doğrulanamadı!", SD_DATA_FILE);
        }
    } else {
        LOG_ERROR("SD Kart kullanılamıyor. Veri kaydı yapılamayacak.");
    }

    if (!bmeSensor.begin()) {
        LOG_ERROR("BME280 sensörü bulunamadı!");
    } else {
        LOG_INFO("BME280 Başarıyla Başlatıldı!");
    }

    npkSensor.begin(); 
    LOG_INFO("NPK Sensörü başlatıldı.");    
    LOG_INFO("Kurulum tamamlandı.");

    npkSensor.factorOffsetReset();
}

void loop() {
    wakeCounter++;
    LOG_INFO("Loop başlatıldı - Uyanma sayısı: %d", wakeCounter);
    commDriver.readAndProcessSolarVoltage();
    commDriver.readAndProcessBatteryVoltage();
    commDriver.updateModemBatteryStatus();
    bmeSensor.readBME280WithRetry(); 
    npkSensor.readNPKWithRetry();
    
    String csvDataRow = commDriver.createCsvDataLine();
    String dataToLog = csvDataRow + "\n"; 

    if (appendFile(SD, SD_DATA_FILE, dataToLog.c_str())) {
        LOG_INFO("Veri başarıyla SD karta kaydedildi: %s", SD_DATA_FILE);
    } else {
        LOG_ERROR("SD karta veri kaydetme başarısız!");
    }

    if (wakeCounter >= MAX_WAKECOUNTER) {
        LOG_INFO("MAX_WAKECOUNTER'a ulaşıldı. Veri göndermeye hazırlanıyor.");

        if (!commDriver.connectMQTT()) {
            LOG_ERROR("MQTT bağlantısı kurulamadı. Veri SD kartta kalacak.");
            goToDeepSleep();
            return;
        }

        String fileContent = readFile(SD, SD_DATA_FILE);

        if (fileContent.length() > 0 && fileContent.indexOf('\n') != fileContent.lastIndexOf('\n')) {
            LOG_INFO("SD karttan veriler okunuyor (%s), tamamı AWS JSON formatında gönderilecek.", SD_DATA_FILE);
            String awsPayload = commDriver.createJsonPayloadForAWS(fileContent);
            
            if (commDriver.publishData(awsPayload.c_str())) {
                LOG_INFO("Veri başarıyla yayınlandı. Sunucu yanıtı için 30 saniye bekleniyor...");
            
                unsigned long listenStart = millis();
                while (millis() - listenStart < 30000) {
                    commDriver.resetLastStatusCode(); // Son durum kodunu sıfırla
                    commDriver.mqttLoop(); // Gelen mesajları işle

                    int statusCode = commDriver.getLastStatusCode();
                    if (statusCode != -1) { // Eğer bir status_code alındıysa
                        LOG_INFO("Sunucudan yanıt alındı. İşlenecek status_code: %d", statusCode);
                        if (statusCode == 0) {
                            LOG_INFO("Status_code 0 alındı: Veri sunucu tarafından onaylandı. Lokal veri siliniyor.");
                            wakeCounter = 0; // Sayaç sıfırlanıyor
                            if (deleteFile(SD, SD_DATA_FILE)) {
                                LOG_INFO("Dosya başarıyla silindi: %s", SD_DATA_FILE);
                                if (writeFile(SD, SD_DATA_FILE, csvHeader)) {
                                    LOG_INFO("Başlık yeni %s dosyasına yazıldı.", SD_DATA_FILE);
                                } else {
                                    LOG_ERROR("%s dosyasına başlık yazılamadı.", SD_DATA_FILE);
                                }
                            } else {
                                LOG_ERROR("Dosya silinemedi: %s", SD_DATA_FILE);
                            }
                            break;
                        } else if (statusCode == 1) {
                            LOG_INFO("Status_code 1 alındı: yanlış hash code, Sunucu tekrar gönderim istedi.");
                            commDriver.publishData(awsPayload.c_str());
                            // Not: Yeniden gönderim sonrası yanıtı bu döngüde beklemiyoruz, bu yüzden döngüden çıkıyoruz.
                        } else if (statusCode == 2) {
                            LOG_INFO("Status_code 2 alındı: float conversion error, Sunucu tekrar gönderim istedi.");
                            commDriver.publishData(awsPayload.c_str());
                        } else if (statusCode == 3) {
                            LOG_INFO("Status_code 3 alındı: not connected to database, Sunucu tekrar gönderim istedi.");
                            commDriver.publishData(awsPayload.c_str());
                        } else if (statusCode == 4) {
                            LOG_INFO("Status_code 4 alındı: not inserted to database, Sunucu tekrar gönderim istedi.");
                            commDriver.publishData(awsPayload.c_str());
                            
                        }  else {
                            LOG_WARN("Bilinmeyen bir status_code (%d) alındı. Lokal veri silinmiyor.", statusCode);
                        }

                        commDriver.resetLastStatusCode(); // İşlem yapıldı, kodu sıfırla
                    }
                    
                }
                LOG_INFO("30 saniyelik dinleme süresi sona erdi.");
                // GPS isteği bayrağını kontrol et
                if (commDriver.gps_request_flag) {
                    LOG_INFO("İşleniyor: GPS isteği.");
                    commDriver.publishGpsData();
                    commDriver.gps_request_flag = false; // Bayrağı sıfırla
                }

                // OTA güncelleme bayrağını kontrol et
                if (commDriver.ota_request_flag) {
                    LOG_INFO("İşleniyor: OTA güncelleme isteği.");
                    commDriver.performOTA(commDriver.ota_url.c_str(), commDriver.ota_version_id.c_str());
                    commDriver.ota_request_flag = false;
                }
                
            } else {
                LOG_ERROR("MQTT'ye AWS JSON gönderimi başarısız. Dosya silinmedi: %s", SD_DATA_FILE);
            }
        } else {
            LOG_INFO("SD kartta gönderilecek yeterli veri yok (%s boş veya sadece başlık içeriyor).", SD_DATA_FILE);
            if (fileContent.length() > 0 && fileContent.indexOf('\n') == fileContent.lastIndexOf('\n') && wakeCounter > 0) {
                LOG_INFO("Dosyada sadece başlık var gibi görünüyor, dosya temizleniyor ve sayaç sıfırlanıyor.");
                wakeCounter = 0;
                 if (deleteFile(SD, SD_DATA_FILE)) {
                    if (writeFile(SD, SD_DATA_FILE, csvHeader)) {
                       LOG_INFO("Başlık yeni %s dosyasına yazıldı.", SD_DATA_FILE);
                    } else {
                       LOG_ERROR("%s dosyasına başlık yazılamadı.", SD_DATA_FILE);
                    }
                }
            }
        }
        delay(1000); 
    }
    LOG_INFO("Derin uyku moduna hazırlanılıyor...");
    goToDeepSleep(); 
}
