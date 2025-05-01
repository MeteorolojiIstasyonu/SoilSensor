#ifndef CONFIG_H
#define CONFIG_H

// /////////////////////////////////////////////////////////////////
// ''''''' GENEL KONFİGÜRASYON ''''''''''''''''''' //

#define BLE_MSG_LENGTH 39  // BLE mesaj uzunluğu (Örnek değer)
// Seri haberleşme loglarını aç/kapat
#define LOG_SERIAL_ENABLED  1

// Seri haberleşme baud hızı
#define SERIAL_BAUD_RATE 115200

// ESP32 UART pinleri (SIM808 için)
#define UART_RX_PIN 16
#define UART_TX_PIN 17
//VOLTAJ PINİ (ADC) için 
#define VOLTAGE_PIN 34 // ADC pin (Pil voltajı ölçümü için)
// Voltaj Ölçüm Ayarları
#define VOLTAGE_PIN 34 // ADC pin (Pil voltajı ölçümü için)
#define VOLTAGE_DIVIDER_R1 1000.0 // VCC ile ADC pini arasındaki direnç (Ohm)  
#define VOLTAGE_DIVIDER_R2 3300.0 // ADC pini ile GND arasındaki direnç (Ohm)  
#define ADC_VREF 3.50             // ADC referans voltajı (V)  
#define ADC_MAX_VALUE 4095.0      // ADC Maksimum Değeri (12-bit için 4095)  

// NPK sensörü pinleri (Modbus için)
#define RXX 25
#define TXX 26
#define DE_RE 27  // RS485 yön kontrol pini

//      BME280 Sensör Tanımları
#define I2C_SDA 21  // ESP32'nin SDA pini
#define I2C_SCL 22  // ESP32'nin SCL pini
#define SEALEVELPRESSURE_HPA (1013.25) // Deniz seviyesindeki varsayılan basınç
#define PWR 5  // BME280 sensörünü açma/kapama pini

// SIM kart PIN numarası (varsa girin, yoksa boş bırakın)
#define SIM_CARD_PIN ""
#define APN "internet" // Operatörünüzün APN'i
#define APN_USERNAME ""
#define APN_PASSWORD ""

// Cihaz ID'si 
#define DEVICE_ID "002"

// ''''''' LOGGER AYARLARI ''''''''''''''''''' //
// ... (Mevcut Logger ayarları) ...
#ifndef CORE_DEBUG_LEVEL
#define CORE_DEBUG_LEVEL 4  // Varsayılan log seviyesi
#endif

#define LOG_LEVEL_SELECTED  CORE_DEBUG_LEVEL // Seçili log seviyesi

// Log seviyeleri
#define LOGGER_LEVEL_ERROR  1
#define LOGGER_LEVEL_WARN   2
#define LOGGER_LEVEL_INFO   3
#define LOGGER_LEVEL_DEBUG  4

// Log mesaj kuyruğu ayarları
#define LOGGER_QUEUE_SIZE  10
#define LOGGER_MAX_LOG_LENGTH  128

// Logger task stack boyutu
#define LOGGER_TASK_STACK_SIZE   16384

// Log mesajlarını ne sıklıkla göstereceğiz? (ms)
#define PRINT_INTERVAL 1000
#define DEFAULT_LOOP_INTERVAL_MS  5000 // Veri gönderme aralığı
#define uykusuresi 30000000  // 30 saniye (30.000.000 mikro saniye)

#ifdef LOG_LEVEL_SELECTED
    #if LOG_LEVEL_SELECTED == LOGGER_LEVEL_DEBUG
        #define LOOP_INTERVAL_MS   DEFAULT_LOOP_INTERVAL_MS  // Debug modunda da aynı kalsın veya ayarlayın
    #else
        #define LOOP_INTERVAL_MS   DEFAULT_LOOP_INTERVAL_MS
    #endif
#else
    #define LOOP_INTERVAL_MS   DEFAULT_LOOP_INTERVAL_MS
#endif

#endif // CONFIG_H