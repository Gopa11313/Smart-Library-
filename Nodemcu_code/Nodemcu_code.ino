#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#define BLYNK_PRINT Serial1
#include <ArduinoJson.h>
#include <BlynkSimpleEsp8266.h>
ESP8266WebServer server;
char auth[] = "FBL4wvXk6QflJyGLQhws34JxZD6EWhWF";
char ssid[] = "noobwifi_wlink";
char password[] = "12345zxcvb";
int buzzer = D2;
int sensorThres = 100;
void setup()
{
  pinMode(buzzer, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.begin(9600);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Blynk.begin(auth, ssid, password);
  server.on("/", handleIndex);
  server.begin();
}

void loop()
{
  server.handleClient();
}

void handleIndex()
{
  // Send a JSON-formatted request with key "type" and value "request"
  // then parse the JSON-formatted response with keys "gas" and "distance"
  DynamicJsonDocument doc(1024);
  double  temp = 0, hum = 0;
  String access = "";
  int distance = 0;
  int gas = 0;
  doc["type"] = "request";
  serializeJson(doc, Serial);
  // Reading the response
  boolean messageReady = false;
  String message = "";
  while (messageReady == false) { // blocking but that's ok
    if (Serial.available()) {
      message = Serial.readString();
      messageReady = true;
    }
  }
  // Attempt to deserialize the JSON-formatted message
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
  temp = doc["temp"];
  gas = doc["gas"];
  hum = doc["hum"];
  access = (const char*)doc["access"];
  distance = doc["distance"];
  // Prepare the data for serving it over HTTP
  String output = "temp: " + String(temp) + "F" + "\n";
  output += "CO level: " + String(gas) + "\n";
  output += "Hum: " + String(hum) + "\n";
  output += "access: " + String(access) + "\n";
  int distancef = int(distance);
  if (distancef <= 10)
  {
    ///----notify user----//
    Blynk.virtualWrite(V3, 255);
    output += "Dust bin is full please clean it...!!";
    Blynk.notify("Empty your Dustbin!!");
  }
  
  if (int(gas) > sensorThres)
  {
    
    tone(buzzer, 1000, 200);
    
  }
  
  // Serve the data as plain text, for example
  server.send(200, "text/plain", output);
  delay(5000);
}
