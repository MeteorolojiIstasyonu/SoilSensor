#include "BME280Sensor.h"
#include <Wire.h>
#include "Config.h"

BME280Sensor::BME280Sensor()
    : temperature(0), humidity(0), pressure(0) {}

bool BME280Sensor::begin(uint8_t address) {
    Wire.begin(I2C_SDA, I2C_SCL);
    return bme.begin(address);
}

bool BME280Sensor::readData() {
    temperature = bme.readTemperature(); // °C
    humidity = bme.readHumidity();       // %
    pressure = bme.readPressure() / 100.0F; // hPa
    Serial.println("----- BME280 Sensör Verileri -----");
    Serial.printf("Hava Sıcaklığı: %.2f °C\n", temperature);
    Serial.printf("Hava Nem: %.1f %%\n", humidity);
    Serial.printf("Hava Basıncı: %.2f hPa\n", pressure);
    return true; // Okuma başarılı sayılır, hata kontrolü istersen buraya eklenebilir
}

