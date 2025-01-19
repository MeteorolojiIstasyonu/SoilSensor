# Project Requirements & Module Specifications

This document outlines the updated and more detailed requirements for each module in the soil-measurement device project. The project includes:

- A device (ESP32 38-pin) inserted into the soil.
- Measurements for soil moisture, temperature, pH, EC, and NPK values.
- Periodic data acquisition and calibration.
- Power management (deep sleep every hour).
- Battery monitoring.
- Data logging on an SD card if no connection.
- Watchdog to reset the device if it becomes unresponsive (including daily resets).
- OTA updates for the firmware.
- Communication to a remote server (ThingSpeak) via SIM808.

Below, each module follows a template with **Description**, **Responsibilities**, **Global Variables**, **Local Variables**, **Parameters**, **Functions**, and **Error Handling**.

---

## 1. ServerConnection Module

### 1.1 Description
Handles connectivity with the ThingSpeak server through the SIM808 modem. Initializes the SIM808 module and sends sensor data using HTTP requests.

### 1.2 Responsibilities
1. Initialize the SIM808 modem (UART pins, baud rate, timeouts).
2. Handle HTTP connections to ThingSpeak.
3. Send sensor data (soil measurements) to the remote server.
4. Return specific error codes on failure.

### 1.3 Global Variables
*None specific to this module at the moment.*  
*(Typically, states like connection status could be stored here if needed.)*

### 1.4 Local Variables
- Internal buffers for AT commands and HTTP requests.
- Internal state flags (e.g., “isConnected”, “networkRegistered”).

### 1.5 Parameters (Preprocessor Definitions)
- **`ServerConnection_Pin_TX_Sim808`** (Default: 17)  
- **`ServerConnection_Pin_RX_Sim808`** (Default: 16)  
- **`ServerConnection_Baudrate_Sim808`** (Default: 115200)  
- **`ServerConnection_TimeoutSim808`** (Default: 10000 ms)  
- **`ServerConnection_ThingSpeakAPIKey`** (String; must be valid)  
- **`ServerConnection_ThingSpeakURL`** (String; valid ThingSpeak URL)

### 1.6 Functions

1. **`int ServerConnection_InitSim808()`**  
   - **Description**:  
     Initializes SIM808 using the defined TX/RX pins and baud rate.  
   - **Behavior**:  
     1. Powers on the SIM808 (if managed by a power pin).  
     2. Sends AT commands to verify communication and SIM presence.  
     3. Waits for network registration (checks signal).  
     4. Returns `0` on success or an error code (see **Error Handling**).

2. **`int ServerConnection_Send_NPK_Data(float temperature, float humidity, float EC, float pH, float nitrogen, float phosphorus, float potassium)`**  
   - **Description**:  
     Builds an HTTP request to send soil sensor data to ThingSpeak.  
   - **Behavior**:  
     1. Validates that SIM808 is initialized and network-registered.  
     2. Sends the data (temperature, humidity, pH, etc.) via HTTP POST/GET.  
     3. Monitors for timeouts (based on `ServerConnection_TimeoutSim808`).  
     4. Returns `0` if successful, or a specific error code otherwise.

### 1.7 Error Handling
Uses the existing global error codes. Possible relevant codes include:

- **1** (`ERR_SIM808_INIT_FAIL`)  
- **2** (`ERR_SIM808_TIMEOUT`)  
- **3** (`ERR_SIM_NOT_INSERTED`)  
- **4** (`ERR_NETWORK_REG_FAIL`)  
- **5** (`ERR_API_KEY_INVALID`)  
- **6** (`ERR_HTTP_CONNECTION_FAIL`)  
- **7** (`ERR_HTTP_SEND_FAIL`)  
- **8** (`ERR_HTTP_RESPONSE_INVALID`)  
- **9** (`ERR_DATA_FORMAT`)  
- **10** (`ERR_UNKNOWN`)  

---

## 2. SoilSensorManager Module

### 2.1 Description
Manages reading data from the **Soil Sensor** via RS485. Conducts periodic calibration and validates data.

### 2.2 Responsibilities
1. Initialize the RS485 communication for the **Soil Sensor**.
2. Read sensor data (moisture, temperature, pH, EC, nitrogen, phosphorus, potassium).
3. Maintain and apply calibration coefficients for the **Soil Sensor**.
4. Validate sensor data integrity/ranges.

### 2.3 Global Variables
- **`SoilSensorManager_calibrationCoefficients[SoilSensorManager_SensorDataCount][SoilSensorManager_CalibrationCoefficientCount]`**  
  *2D array storing calibration coefficients for each of the sensor data types. The first dimension represents each sensor value (length = `SoilSensorManager_SensorDataCount`), and the second dimension represents the number of coefficients (length = `SoilSensorManager_CalibrationCoefficientCount`).*
- **`SoilSensorManager_sensorValues[SoilSensorManager_SensorDataCount]`**  
  *Array storing the latest read values from the Soil Sensor.*

### 2.4 Local Variables
- Internal buffers for RS485 transactions.
- Status flags (e.g., “isCalibrated”).
- Temporary parsing structures (CRC checks, response buffers, etc.).

### 2.5 Parameters (Preprocessor Definitions or Configuration)
- **`SoilSensorManager_Pin_RS485_RX`**
- **`SoilSensorManager_Pin_RS485_TX`**
- **`SoilSensorManager_RS485_REDE_Pin`**  
- **`SoilSensorManager_Baudrate_RS485`** (Default can be 9600 or 19200)
- **`SoilSensorManager_SensorReadInterval`** (Default: 1 hour, or define as needed)
- **`SoilSensorManager_CalibrationCoefficientCount`** (Default: 2)
- **`SoilSensorManager_SensorDataCount`** (Default: 7)

### 2.6 Functions

1. **`int SoilSensorManager_InitRS485()`**  
   - **Description**:  
     Configures UART for RS485 and sets DE/RE pins.  
   - **Behavior**:  
     1. Sets up RX/TX pins.  
     2. Sets the `SoilSensorManager_RS485_REDE_Pin` to receive mode by default.  
     3. Returns `0` on success, or error code if communication fails.

2. **`int SoilSensorManager_ReadSoilData(float* temperature, float* moisture, float* pH, float* EC, float* nitrogen, float* phosphorus, float* potassium)`**  
   - **Description**:  
     Requests data from the Soil Sensor via RS485 and populates the provided pointers.  
   - **Behavior**:  
     1. Sends a request frame to the sensor.  
     2. Switches to receive mode and reads the response.  
     3. Parses the data into the correct format and validates.  
     4. Returns `0` on success, or error code if a communication/parsing error occurs.

3. **`int SoilSensorManager_CalibrateSensors()`** *(Optional)*  
   - **Description**:  
     Applies or updates calibration data in `SoilSensorManager_calibrationCoefficients`.  
   - **Behavior**:  
     1. Either uses known references or user-provided offsets/slopes.  
     2. Updates the global calibration array.  
     3. Returns `0` on success, or an error code on failure.

### 2.7 Error Handling
Relevant error codes might include:
- **11** (`ERR_RS485_COMM_FAIL`)
- **12** (`ERR_SENSOR_CALIBRATION`)
- **9** (`ERR_DATA_FORMAT`)
- **10** (`ERR_UNKNOWN`)

---

## 3. SleepManager Module

### 3.1 Description
Responsible for managing the ESP32 deep sleep cycle: sleeps for an hour, wakes up to perform tasks (sensor reading, data sending), and goes back to sleep.

### 3.2 Responsibilities
1. Configure deep sleep intervals (1 hour).
2. Manage wake-up sources and triggers.
3. Integrate with the Watchdog to avoid indefinite wakefulness.

### 3.3 Global Variables
*None specifically required. State can be local unless persistent across boots is needed.*

### 3.4 Local Variables
- Internal time calculations for next sleep cycle.

### 3.5 Parameters
- **`SleepManager_SleepInterval`** (Default: 1 hour in microseconds)
- **`SleepManager_WakePin`** (Optional, if external wake is used)

### 3.6 Functions

1. **`void SleepManager_ConfigureDeepSleep(unsigned long duration_us)`**  
   - **Description**:  
     Sets up and enters ESP32 deep sleep for `duration_us`.  
   - **Behavior**:  
     1. Configures wake-up sources.  
     2. Calls `esp_deep_sleep_start()`.  

2. **`bool SleepManager_IsWakeFromDeepSleep()`**  
   - **Description**:  
     Determines if the current boot is from deep sleep or a fresh reset.  
   - **Behavior**:  
     - Returns `true` if the device woke from deep sleep; `false` otherwise.

### 3.7 Error Handling
- **13** (`ERR_SLEEP_CONFIG`) if invalid parameters or configuration.

---

## 4. BatteryMonitor Module

### 4.1 Description
Monitors the battery voltage every hour using the ESP32 ADC.

### 4.2 Responsibilities
1. Initialize ADC for battery measurement.
2. Convert ADC values to actual battery voltage.
3. Potentially estimate battery percentage.

### 4.3 Global Variables
*None specific to this module by default.*

### 4.4 Local Variables
- Internal calibration offset for ADC (if needed).
- Temporary storage for last read battery voltage.

### 4.5 Parameters
- **`BatteryMonitor_Pin_BatterySense`** (ADC pin)
- **`BatteryMonitor_VoltageDividerFactor`** (Default ratio if using a voltage divider)

### 4.6 Functions

1. **`int BatteryMonitor_Init()`**  
   - **Description**:  
     Sets up the ADC to read the battery voltage.  
   - **Behavior**:  
     1. Configures the ADC channel.  
     2. Returns `0` on success, or an error code otherwise.

2. **`float BatteryMonitor_GetVoltage()`**  
   - **Description**:  
     Reads the ADC and converts it to actual battery voltage.  
   - **Behavior**:  
     1. Takes raw ADC reading.  
     2. Multiplies by `BatteryMonitor_VoltageDividerFactor`.  
     3. Returns the voltage as a float.

### 4.7 Error Handling
- **14** (`ERR_ADC_READ_FAIL`) if the ADC read fails.

---

## 5. DataLogger Module

### 5.1 Description
Handles all data logging to the SD card and console. It also manages file/folder creation, size limits, and offline data storage for soil sensor and battery data.

### 5.2 Responsibilities
1. Initialize the SD card for file operations.
2. Log diagnostic messages based on a set log level.
3. Write and read **Soil Sensor** data when offline or for archival.
4. Write and read battery data.
5. Manage log files (rotation, deletion) when exceeding the size threshold.

### 5.3 Global Variables
*None by default.*  
*(Most state can be kept local or within log structures.)*

### 5.4 Local Variables
- Internal file handles.
- Buffer for log messages.

### 5.5 Parameters (Preprocessor Definitions or Configuration)
- **`DataLogger_Pin_SD_CS`**: Chip select for the SD card.  
- **`DataLogger_LogLevel`**: Global log level threshold.  
- **`DataLogger_BatteryMonitorFilePath`** (Default: `"Battery"`)  
- **`DataLogger_SoilSensorDataFilePath`** (Default: `"SoilSensor/Data"`)  
- **`DataLogger_SoilSensorCoefficientsFilePath`** (Default: `"SoilSensor/Coeff"`)  
- **`DataLogger_LogFilePath`** (Default: `"Logs"`)  
- **`DataLogger_MaxLogFolderSize`** (Default: `"10GB"`)  
- **`DataLogger_MaxLogFileSize`** (Existing from previous examples or define as needed.)

> **Folder & File Creation Requirement**  
> Every function that writes to a file must:
> - Check if the folder path exists. If not, create it.
> - Check if the file exists. If not, create it.

### 5.6 Functions

1. **`int DataLogger_InitSDCard()`**  
   - **Description**:  
     Initializes the SD card.  
   - **Behavior**:  
     1. Verifies card presence, mounts file system.  
     2. Returns `0` on success, or an error code if it fails.

2. **`void DataLogger_LogMessage(int level, const char* message)`**  
   - **Description**:  
     Logs a message if `level >= DataLogger_LogLevel`. Also writes to SD (in the folder specified by `DataLogger_LogFilePath`).  
   - **Behavior**:  
     1. Checks if the log folder exists. If not, create it.  
     2. If log file doesn’t exist, create it (new file on each restart if logs exceed threshold).  
     3. Prints to Serial if `level >= DataLogger_LogLevel`.  
     4. Writes the message to the log file.  
     5. If log folder size > `DataLogger_MaxLogFolderSize`, handle rotation/deletion.

3. **`int DataLogger_WriteSoilSensorData(float temperature, float humidity, float EC, float pH, float nitrogen, float phosphorus, float potassium)`**  
   - **Description**:  
     Writes soil sensor data to a file in the SD card (path from `DataLogger_SoilSensorDataFilePath`).  
   - **Behavior**:  
     1. Creates folder/file if they do not exist.  
     2. Appends a timestamp and sensor values (CSV, JSON, etc.).  
     3. Returns `0` on success, or an error code if writing fails.

4. **`int DataLogger_ReadSoilSensorData()`**  
   - **Description**:  
     Reads soil sensor data file from `DataLogger_SoilSensorDataFilePath`.  
   - **Behavior**:  
     1. Checks if file/folder exist.  
     2. Reads content (could load all or read line by line).  
     3. Returns data or an error code.

5. **`int DataLogger_UpdateSoilSensorCoefficients(float newCoeffs[][2], int sensorCount, int coeffCount)`**  
   - **Description**:  
     Updates the existing soil sensor calibration coefficients on the SD card.  
   - **Behavior**:  
     1. Creates folder/file if needed.  
     2. Writes the array to the `DataLogger_SoilSensorCoefficientsFilePath`.  
     3. Returns `0` on success, or error code.

6. **`int DataLogger_ReadSoilSensorCoefficients(float coeffsOut[][2], int sensorCount, int coeffCount)`**  
   - **Description**:  
     Reads calibration coefficients from the SD card.  
   - **Behavior**:  
     1. Checks if file/folder exist.  
     2. Reads data and populates `coeffsOut`.  
     3. Returns `0` on success, or an error code if reading fails.

7. **`int DataLogger_WriteBatteryData(float voltage)`**  
   - **Description**:  
     Appends battery voltage data to the file in `DataLogger_BatteryMonitorFilePath`.  
   - **Behavior**:  
     1. Checks/creates folder/file.  
     2. Writes voltage with timestamp.  
     3. Returns `0` on success, or error code.

8. **`int DataLogger_ReadBatteryData()`**  
   - **Description**:  
     Reads battery voltage history from `DataLogger_BatteryMonitorFilePath`.  
   - **Behavior**:  
     1. Checks/creates folder if needed (in case it was never used).  
     2. Reads battery data from the file.  
     3. Returns the data or an error code.

9. **`int DataLogger_WriteLogs(int level, const char* message)`**  
   - **Description**:  
     Similar to `DataLogger_LogMessage` but enforces creating a **new file at every restart** if volume is exceeded.  
   - **Behavior**:  
     1. Checks `DataLogger_LogFilePath`. If folder doesn’t exist, create it.  
     2. If file doesn’t exist or a new file is required, create it.  
     3. Writes log data with timestamp.  
     4. Returns `0` if success, or an error code.

10. **`unsigned long DataLogger_MeasureLogVolume()`**  
    - **Description**:  
      Measures total size of the logs folder to compare against `DataLogger_MaxLogFolderSize`.  
    - **Behavior**:  
      1. Recursively sums file sizes in `DataLogger_LogFilePath`.  
      2. Returns the size in bytes.

11. **`int DataLogger_DeleteLogs()`**  
    - **Description**:  
      Deletes older log files or entries when the log folder exceeds `DataLogger_MaxLogFolderSize`.  
    - **Behavior**:  
      1. Identifies oldest logs.  
      2. Removes them until size is below threshold.  
      3. Returns `0` on success.

### 5.7 Error Handling
- **15** (`ERR_SD_INIT_FAIL`)  
- **16** (`ERR_SD_WRITE_FAIL`)  
- **17** (`ERR_SD_READ_FAIL`)  
- **10** (`ERR_UNKNOWN`)

---

## 6. WatchdogManager Module

### 6.1 Description
Implements a watchdog timer to reset the ESP32 if it becomes unresponsive and schedules a daily restart.

### 6.2 Responsibilities
1. Configure the watchdog timer and feed it regularly.
2. Automatically reset the device if the watchdog is not fed in time.
3. Perform a forced daily restart of the ESP32.

### 6.3 Global Variables
*None needed unless storing last reset time, etc.*

### 6.4 Local Variables
- Internal timer tracking for daily resets.

### 6.5 Parameters
- **`WatchdogManager_WDT_TIMEOUT_MS`** (Default: 30000 ms)
- **`WatchdogManager_WDT_TASK_INTERVAL_MS`** (Time between feeding)

### 6.6 Functions

1. **`int WatchdogManager_InitWatchdog(unsigned long timeout_ms)`**  
   - **Description**:  
     Initializes the hardware/software watchdog timer with the specified timeout.  
   - **Behavior**:  
     1. Sets registers or configures the ESP32 watchdog.  
     2. Returns `0` if successful, or an error code if it fails.

2. **`void WatchdogManager_FeedWatchdog()`**  
   - **Description**:  
     Resets the watchdog timer countdown to prevent a reset.  
   - **Behavior**:  
     - Called periodically or in main loop tasks.

3. **`void WatchdogManager_ForceDailyRestart()`**  
   - **Description**:  
     Triggers a forced restart every 24 hours (or at a configured time).  
   - **Behavior**:  
     1. Tracks the last reset time.  
     2. If 24 hours have elapsed, performs a software or hardware reset.

### 6.7 Error Handling
- **18** (`ERR_WDT_INIT_FAIL`)  
*(Resets are typically handled by hardware if the watchdog is not fed.)*

---

## 7. OTAUpdater Module

### 7.1 Description
Manages Over-The-Air updates for the ESP32 firmware, including checking for new versions, downloading, validating, and flashing.

### 7.2 Responsibilities
1. Periodically check for new firmware at a given URL.
2. Download firmware securely.
3. Validate firmware integrity (checksum/signature).
4. Flash new firmware and reboot if successful.

### 7.3 Global Variables
*None by default.*

### 7.4 Local Variables
- Temporary buffer for downloaded firmware.
- State flags (e.g., “updateInProgress”).

### 7.5 Parameters
- **`OTAUpdater_OTA_SERVER_URL`**  
- **`OTAUpdater_OTA_CHECK_INTERVAL`**  
- **`OTAUpdater_OTA_SECURE`** (boolean, use HTTPS or not)

### 7.6 Functions

1. **`int OTAUpdater_InitOTA()`**  
   - **Description**:  
     Sets up the device for OTA (network or Wi-Fi / SIM808 usage).  
   - **Behavior**:  
     - Returns `0` on success or error code if not possible (e.g., memory partition issues).

2. **`int OTAUpdater_CheckForUpdates()`**  
   - **Description**:  
     Compares the current firmware version with the version available at `OTAUpdater_OTA_SERVER_URL`.  
   - **Behavior**:  
     - Returns `0` if no update is available, or a special code if an update is ready, or an error code on failure.

3. **`int OTAUpdater_PerformOTAUpdate()`**  
   - **Description**:  
     Downloads and flashes the new firmware.  
   - **Behavior**:  
     1. Downloads binary from `OTAUpdater_OTA_SERVER_URL`.  
     2. Verifies integrity (checksum/signature).  
     3. Writes to the OTA partition.  
     4. Reboots on success.  
     5. Returns `0` if successful, or an error code otherwise.

### 7.7 Error Handling
- **19** (`ERR_OTA_INIT_FAIL`)  
- **20** (`ERR_OTA_DOWNLOAD_FAIL`)  
- **21** (`ERR_OTA_INVALID_FIRMWARE`)  
- **22** (`ERR_OTA_FLASH_FAIL`)

---

## Expanded Error Codes Reference

| Code | Name                             | Definition                                                          |
|------|----------------------------------|---------------------------------------------------------------------|
| **0**  | NO_ERROR                         | Everything executed successfully.                                   |
| **1**  | ERR_SIM808_INIT_FAIL             | SIM808 did not respond to initialization (ServerConnection).        |
| **2**  | ERR_SIM808_TIMEOUT               | SIM808 timed out (ServerConnection).                                |
| **3**  | ERR_SIM_NOT_INSERTED             | SIM card missing or unreadable (ServerConnection).                  |
| **4**  | ERR_NETWORK_REG_FAIL             | SIM808 failed to register on network (ServerConnection).            |
| **5**  | ERR_API_KEY_INVALID              | Invalid ThingSpeak API key (ServerConnection).                      |
| **6**  | ERR_HTTP_CONNECTION_FAIL         | Cannot connect to ThingSpeak (ServerConnection).                    |
| **7**  | ERR_HTTP_SEND_FAIL               | Failed to send HTTP request (ServerConnection).                     |
| **8**  | ERR_HTTP_RESPONSE_INVALID        | Invalid or incomplete server response (ServerConnection).           |
| **9**  | ERR_DATA_FORMAT                  | Sensor data out of range or incorrect format.                       |
| **10** | ERR_UNKNOWN                      | An unspecified error occurred.                                      |
| **11** | ERR_RS485_COMM_FAIL              | RS485 communication error (SoilSensorManager).                      |
| **12** | ERR_SENSOR_CALIBRATION           | Sensor calibration failed (SoilSensorManager).                      |
| **13** | ERR_SLEEP_CONFIG                 | Invalid sleep configuration (SleepManager).                         |
| **14** | ERR_ADC_READ_FAIL                | ADC read failed (BatteryMonitor).                                   |
| **15** | ERR_SD_INIT_FAIL                | SD card initialization error (DataLogger).                          |
| **16** | ERR_SD_WRITE_FAIL               | Failed to write to SD card (DataLogger).                            |
| **17** | ERR_SD_READ_FAIL                | Failed to read from SD card (DataLogger).                           |
| **18** | ERR_WDT_INIT_FAIL               | Watchdog initialization failure (WatchdogManager).                  |
| **19** | ERR_OTA_INIT_FAIL               | OTA environment initialization failed (OTAUpdater).                 |
| **20** | ERR_OTA_DOWNLOAD_FAIL           | OTA firmware download failed (OTAUpdater).                          |
| **21** | ERR_OTA_INVALID_FIRMWARE        | OTA firmware checksum/signature mismatch (OTAUpdater).              |
| **22** | ERR_OTA_FLASH_FAIL              | Error writing firmware to flash (OTAUpdater).                       |

---

# Final Notes

By following the structure above, each module is clearly separated, with well-defined **global and local variables**, **parameters**, and **functions**. The consistent use of **error codes** ensures that all parts of the system can properly handle and log issues.

1. **Order of Initialization**: Typically, the system will:
   1. Initialize the **Watchdog**.  
   2. Initialize **DataLogger** (SD card).  
   3. Initialize **BatteryMonitor**, **SoilSensorManager**, and **ServerConnection**.  
   4. Attempt data read + send or store offline.  
   5. Check for OTA updates.  
   6. Enter deep sleep via **SleepManager**.  

2. **Daily Reset**: The **WatchdogManager** enforces a forced daily reset, ensuring the system remains stable over long periods.

3. **Data & Calibration**: The **SoilSensorManager** uses global arrays for sensor values and calibration. The **DataLogger** can update/read them from the SD card as needed.

4. **Logging**: The **DataLogger** module ensures logs and sensor data are always stored properly, creating folders/files if they don’t exist. It also handles log file rotation upon reaching size limits.

5. **OTA**: The **OTAUpdater** allows remote firmware updates, ensuring the device can receive new features or patches with minimal user intervention.

This modular, detailed approach guarantees easier maintenance, debugging, and future expansion of the project.


