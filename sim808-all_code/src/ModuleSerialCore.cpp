#include "ModuleSerialCore.h"

ModuleSerialCore::ModuleSerialCore(HardwareSerial &serialPort, int rxPin, int txPin) 
    : serial(serialPort) 
{
    serial.begin(115200, SERIAL_8N1, rxPin, txPin);
    LOG_INFO("ModuleSerialCore başlatıldı. RX: %d, TX: %d", rxPin, txPin);
}

void ModuleSerialCore::debug(HardwareSerial *printer)
{
    this->printer = printer;
}

int ModuleSerialCore::begin(int baudRate)
{
    serial.begin(baudRate);
    LOG_INFO("Seri port başlatılıyor, Baud Rate: %d", baudRate);

    if (!writeCommand("AT", "OK", 2000)) {
        LOG_ERROR("AT komutuna yanıt alınamadı! Modül başarısız.");
        return MODULE_FAIL;
    }

    if (printer != nullptr) {
        writeCommand("AT+CMEE=1", "OK", 2000); // Ayrıntılı hata raporlama açıldı
    }

    LOG_INFO("Modül başlatıldı ve hazır.");
    return MODULE_READY;
}

int ModuleSerialCore::isReady()
{
    bool status = writeCommand("AT", "OK", 2000);
    LOG_DEBUG("Modül hazır mı? %s", status ? "EVET" : "HAYIR");
    return status ? MODULE_READY : MODULE_FAIL;
}

bool ModuleSerialCore::writeCommand(const char *command, const char *expected, unsigned long timeout)
{
    serial.flush();
    serial.println(command);
    LOG_DEBUG("Komut gönderildi: %s", command);

    char response[200] = "";
    int i = 0;
    unsigned long start = millis();

    while ((millis() - start) < timeout)
    {
        while (serial.available())
        {
            response[i++] = serial.read();
            if (i >= sizeof(response) - 1)
            {
                response[i] = '\0';
                LOG_WARN("Yanıt tampona sığmadı! Yanıt kesilmiş olabilir.");
                return false;
            }
        }
    }

    response[i] = '\0';
    if (printer != nullptr)
    {
        printer->println(response);
    }

    bool result = strstr(response, expected) != nullptr;
    LOG_DEBUG("Gelen yanıt: %s | Beklenen: %s | Sonuç: %s", response, expected, result ? "BAŞARILI" : "BAŞARISIZ");

    return result;
}

void ModuleSerialCore::writeCommand(const char *command, char *output, int size, unsigned long timeout)
{
    serial.flush();
    serial.println(command);
    LOG_DEBUG("Komut gönderildi ve yanıt bekleniyor: %s", command);

    int i = 0;
    unsigned long start = millis();

    while ((millis() - start) < timeout)
    {
        while (serial.available())
        {
            output[i++] = serial.read();
            if (i >= size - 1)
            {
                output[i] = '\0';
                LOG_WARN("Yanıt tampona sığmadı! Yanıt kesilmiş olabilir.");
                return;
            }
        }
    }

    output[i] = '\0';

    if (printer != nullptr)
    {
        printer->println(output);
    }

    LOG_DEBUG("Gelen yanıt: %s", output);
}
