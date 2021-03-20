#include <ArduinoJson.h>
#include <DHT.h>;
String message = "";
bool messageReady = false;
#define DHTPIN 2
#define DHTTYPE DHT22

#define echoPin 3
#define trigPin 4

long duration;
int distance;

#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);

DHT dht(DHTPIN, DHTTYPE);
void setup() {

  Serial.begin(9600);
  dht.begin();
  SPI.begin();      // Initiate  SPI bus
  mfrc522.PCD_Init();
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT);
}

void loop() {
  // Monitor serial communication
  while (Serial.available()) {
    message = Serial.readString();
    messageReady = true;
  }
  // Only process message if there's one
  if (messageReady) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }
    if (doc["type"] == "request") {
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distance = duration * 0.034 / 2;
      doc["type"] = "response";
      float hum = dht.readHumidity();
      float temp = dht.readTemperature(true);


      String acc = "";
      String content = "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++)
      {
        content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      content.toUpperCase();
      if (content.substring(1) == "BD 31 15 2B") //change here the UID of the card/cards that you want to give access
      {
        acc = "Access denied";
        //        Serial.println("Authorized access");
        delay(2000);
      }

      else   {
        acc = "Authorized access";

        delay(2000);
      }
      doc["temp"] = temp;
      doc["hum"] = hum;
      doc["gas"] = analogRead(A1);
      doc["distance"] = distance;
      doc["access"] = acc;

      serializeJson(doc, Serial);
    }
    messageReady = false;
  }
}
