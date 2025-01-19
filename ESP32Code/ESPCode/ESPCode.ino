

#include <WiFi.h>
#include <HTTPClient.h>



const char* ssid = "Hotspot";
const char* password = "12345678";
const char* serverName = "http://api.thingspeak.com/update";
String apiKey = "KKTQVCPE8072Z8H4";


void setup() {

String t = "1"; 
Serial.begin(115200);
WiFi.begin(ssid,password);
 Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");

  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  char buffer[20] = "";
  if(Serial.available()>0){
    //char data = Serial.read();
    Serial.readBytesUntil('\n',buffer,20);
    Serial.println(buffer);
    }
  
String t = "1"; 

if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
      delay(1000); // wait for 10 seconds
      http.begin(client, serverName);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      String httpRequestData = "api_key=" + apiKey + "&field1=" + String(t);          
      int httpResponseCode = http.POST(httpRequestData);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}
