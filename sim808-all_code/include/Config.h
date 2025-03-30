#ifndef CONFIG_H
#define CONFIG_H

// /////////////////////////////////////////////////////////////////
// ''''''' GENEL KONFİGÜRASYON ''''''''''''''''''' //


#define BLE_MSG_LENGTH 39  // BLE mesaj uzunluğu (Örnek değer)


// Seri haberleşme loglarını aç/kapat
#define LOG_SERIAL_ENABLED  1  

// Seri haberleşme baud hızı
#define SERIAL_BAUD_RATE 115200

// ESP32 UART pinleri
#define UART_RX_PIN 16  
#define UART_TX_PIN 17  

//npk sensörü pinleri
#define RXX 25  
#define TXX 26   
#define DE_RE 4  // RS485 yön kontrol pini

// GPS & GSM modülü baud hızı
#define GPS_GSM_BAUDRATE 9600

// SIM kart PIN numarası (varsa girin, yoksa boş bırakın)
#define SIM_CARD_PIN ""
#define APN "internet"
#define APN_USERNAME ""
#define APN_PASSWORD ""

// Mesaj gönderilecek telefon numarası
#define REMOTE_PHONE "+905518325605"

// /////////////////////////////////////////////////////////////////
// ''''''' LOGGER AYARLARI ''''''''''''''''''' //
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

// /////////////////////////////////////////////////////////////////
// ''''''' WATCHDOG & LOOP AYARLARI ''''''''''''''''''' //
#define WATCHDOG_PERIOD_MS    3000  
#define DEFAULT_LOOP_INTERVAL_MS  20  

#ifdef LOG_LEVEL_SELECTED
    #if LOG_LEVEL_SELECTED == LOGGER_LEVEL_DEBUG
        #define LOOP_INTERVAL_MS   500  // Debug modunda yavaşlat
    #else
        #define LOOP_INTERVAL_MS   DEFAULT_LOOP_INTERVAL_MS  
    #endif
#else
    #define LOOP_INTERVAL_MS   DEFAULT_LOOP_INTERVAL_MS  
#endif

#endif // CONFIG_H
