/**********************************************************
 *  Complete Example: ESP32 (38-pin) + SIM808 + ThingSpeak
 *  Demonstration code with debugging prints and reading 
 *  SIM808 response for verification.
 **********************************************************/

// --------- User Configurations -----------  
const char* APN_NAME      = "internet";        // Replace with your SIM card's APN
const char* THINGSPEAKKEY = "UROYL2M8FVCGG1ID"; // Your ThingSpeak Write API Key
const long  SEND_INTERVAL = 10000;              // Send every 10 seconds (for demo)
// -----------------------------------------

#include <Arduino.h>

// Create a HardwareSerial for SIM808 on UART2 
// TX2 = GPIO17, RX2 = GPIO16 for an ESP32 Dev module
HardwareSerial sim808(2);

//HCSR04 
int trigPins[4] = {4, 27, 12, 0};
int echoPins[4] = {15, 14, 13, 2};
float sure[4], mesafe[4];
//POTANSİYOMETRE
int potPins[2] = {36, 39};
int potValues[2];

// Forward declarations
void initializeSIM808();
bool connectToGPRS();
String sendATCommand(const String& cmd, unsigned long timeout = 1000);
void sendDataToThingSpeak();

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[Setup] Starting...");

  // Initialize SIM808 serial
  sim808.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("[Setup] SIM808 serial initialized at 9600 baud");

  delay(2000);

  // Initialize SIM808 (basic checks)
  initializeSIM808();

  // Connect to GPRS
  if (!connectToGPRS()) {
    Serial.println("[Setup] GPRS connection failed. Check your APN or SIM card.");
  } else {
    Serial.println("[Setup] GPRS connected successfully!");
  }

  Serial.println("[Setup] Setup complete!\n");

  // Initialize sensor pins
  for (int i = 0; i < 4; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }
}

void loop() {
  
  mesafeOlc();
  potOkuma();

  // Send data to ThingSpeak periodically
  sendDataToThingSpeak();

  delay(3000); // Bir sonraki okumayı bekle
}


// Ultrasonic sensor 
void mesafeOlc() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(trigPins[i], LOW);
    delayMicroseconds(5);
    digitalWrite(trigPins[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPins[i], LOW);

    sure[i] = pulseIn(echoPins[i], HIGH);
    mesafe[i] = sure[i] * 0.03432 / 2;
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" Mesafe: ");
    Serial.print(mesafe[i], 1);
    Serial.println(" cm");
    delay(100);
  }
}

// Potentsiyometre
void potOkuma() {
  for (int i = 0; i < 2; i++) {
    potValues[i] = analogRead(potPins[i]);
    Serial.print("Potansiyometre ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(potValues[i]);
  }
}

// Function to send data to ThingSpeak via HTTP GET
void sendDataToThingSpeak() {
  // Construct the URL with sensor values
  String url = "http://api.thingspeak.com/update?api_key=" 
               + String(THINGSPEAKKEY) 
               + "&field1=" + String(mesafe[0], 1)   // HCSR Sensor 1
               + "&field2=" + String(mesafe[1], 1)   // HCSR Sensor 2
               + "&field3=" + String(mesafe[2], 1)   // HCSR Sensor 3
               + "&field4=" + String(mesafe[3], 1)   // HCSR Sensor 4
               + "&field5=" + String(potValues[0])   // Potansiyometre 1
               + "&field6=" + String(potValues[1]);  // Potansiyometre 2

  Serial.println("[ThingSpeak] Preparing to send data via HTTP GET...");

  // Initialize HTTP
  sendATCommand("AT+HTTPINIT");

  // Set HTTP parameters (URL)
  String cmd = "AT+HTTPPARA=\"URL\",\"" + url + "\"";
  sendATCommand(cmd);

  // Start HTTP GET
  String response = sendATCommand("AT+HTTPACTION=0", 5000);

  // Read the HTTP response
  Serial.println("[ThingSpeak] Reading server response...");
  response = sendATCommand("AT+HTTPREAD", 3000);
  Serial.print("[ThingSpeak] HTTP Response: ");
  Serial.println(response);

  // Terminate HTTP connection
  sendATCommand("AT+HTTPTERM");

  // Debug: Print data sent to ThingSpeak
  Serial.print("[ThingSpeak] Data sent: ");
  Serial.print(url);
  Serial.println("\n");
}

// Function to send AT commands to SIM808 and receive response
String sendATCommand(const String& cmd, unsigned long timeout) {
  while (sim808.available()) {
    sim808.read();
  }

  sim808.println(cmd);
  Serial.print("[SIM808 >>] ");
  Serial.println(cmd);

  String response = readSimResponse(timeout);
  Serial.print("[SIM808 <<] ");
  Serial.println(response);

  return response;
}

// Function to read SIM808 responses
String readSimResponse(unsigned long timeout) {
  unsigned long startMillis = millis();
  String response = "";

  while (millis() - startMillis < timeout) {
    while (sim808.available()) {
      char c = sim808.read();
      response += c;
    }
    delay(10); // Allow buffer to fill
  }
  return response;
}

// Function to initialize SIM808
void initializeSIM808() {
  sendATCommand("ATE0"); // Turn off echo
  sendATCommand("AT+CPIN?"); // Check SIM card status
  sendATCommand("AT+CREG?"); // Check network registration
  delay(3000);
}
  
// Function to connect to GPRS
bool connectToGPRS() {
  sendATCommand("AT+SAPBR=0,1", 3000); // Close any existing bearer
  sendATCommand("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); // Set GPRS connection type
  String cmd = "AT+SAPBR=3,1,\"APN\",\"" + String(APN_NAME) + "\"";
  sendATCommand(cmd);
  String response = sendATCommand("AT+SAPBR=1,1", 10000); // Open GPRS context

  if (response.indexOf("OK") == -1) {
    return false; // GPRS failed to open
  }

  response = sendATCommand("AT+SAPBR=2,1", 3000); // Query bearer status
  return response.indexOf("STATE: IP GPRSACT") != -1;
}
