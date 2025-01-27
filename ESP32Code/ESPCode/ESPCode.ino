#include <WiFi.h>
#include <HTTPClient.h>

// WiFi credentials
const char* ssid = "Hotspot";
const char* password = "12345678";

// ThingSpeak API settings
const String apiKey = "KKTQVCPE8072Z8H4";       // Replace with your ThingSpeak API key
const String server = "http://api.thingspeak.com";
const String channelField1 = "field1";      // Field for Party 1 votes
const String channelField2 = "field2";      // Field for Party 2 votes
const String channelField3 = "field3";      // Field for Party 3 votes

// Serial communication settings
#define RX_PIN 16 // ESP32 Rx pin connected to Arduino Tx
#define TX_PIN 17 // ESP32 Tx pin connected to Arduino Rx
HardwareSerial SerialFromArduino(2); // Use UART2 for serial communication with Arduino

int party_1_count = 0;
int party_2_count = 0;
int party_3_count = 0;

void setup() {
  // Initialize serial for debugging and communication with Arduino
  Serial.begin(9600);
  SerialFromArduino.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check for data from Arduino
  if (SerialFromArduino.available()) {
    String receivedData = SerialFromArduino.readStringUntil('\n');
    Serial.println("Received data: " + receivedData);

    // Parse the received data
    if (parseVoteCounts(receivedData)) {
      // Send data to ThingSpeak
      if (sendDataToThingSpeak()) {
        Serial.println("Data successfully uploaded to ThingSpeak.");
      } else {
        Serial.println("Failed to upload data to ThingSpeak.");
      }
    }
  }
}

// Parse vote counts received from Arduino
bool parseVoteCounts(String data) {
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);

  if (firstComma > 0 && secondComma > firstComma) {
    party_1_count = data.substring(0, firstComma).toInt();
    party_2_count = data.substring(firstComma + 1, secondComma).toInt();
    party_3_count = data.substring(secondComma + 1).toInt();

    Serial.println("Parsed Data:");
    Serial.println("Party 1 Count: " + String(party_1_count));
    Serial.println("Party 2 Count: " + String(party_2_count));
    Serial.println("Party 3 Count: " + String(party_3_count));
    return true;
  }

  Serial.println("Error parsing data.");
  return false;
}

// Send data to ThingSpeak
bool sendDataToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
    delay(5000);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Failed to reconnect to WiFi.");
      return false;
    }
  }

  HTTPClient http;
  String url = server + "/update?api_key=" + apiKey + 
               "&" + channelField1 + "=" + String(party_1_count) +
               "&" + channelField2 + "=" + String(party_2_count) +
               "&" + channelField3 + "=" + String(party_3_count);

  http.begin(url);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    http.end();
    return true;
  } else {
    Serial.println("Error on sending GET request.");
    http.end();
    return false;
  }
}
