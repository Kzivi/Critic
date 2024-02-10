#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "Arduino.h"

#define EAP_IDENTITY "" //wifi identity
#define EAP_USERNAME "" //wifi login
#define EAP_PASSWORD "" //wifi password

bool playedOnce = false;
const char *ssid = "";  //wifi ssid
const int ledPin = 2; //eps32 onboard test led
const int RED_PIN = 26; //Red led pin
const int BLU_PIN = 25; //blue led pin
const int potentiometerPin = 15;  // Pin D15 for Potentiometer from cheap servomotor
int SMXPin = 18;            // Pin D2 for motor
int SMYPin = 19;            // Pin D4 for motor
const int Buzzer = 21;  //buzzer pin
int potValue;

int counter = 0;

const char *jiraBaseUrl = "https://jira.domain.com"; //jira base url
const char *patToken = ""; //jira token

int total = 0;
const char *jiraEmail = ""; //jira login

bool isInternetConnected() {
     WiFiClient client;
     return client.connect("google.com", 80); //testing connection
}

void closeConnection(HTTPClient &http) {
     http.end();
}

void checkJiraAndBlink() {
     HTTPClient http;
     String jiraApiUrl = String(jiraBaseUrl) +
"/rest/gadget/[..]"; //URL to get value of prepered filter
     http.begin(jiraApiUrl);
     http.addHeader("Authorization", "Bearer " + String(patToken));
     int httpResponseCode = http.GET();
     if (httpResponseCode == HTTP_CODE_OK) {
         String payload = http.getString();
         DynamicJsonDocument doc(6144);
         DeserializationError error = deserializeJson(doc, payload);
         if (error) {
             Serial.print("Error JSON: ");
             Serial.println(error.c_str());
             return;
         }
         total = doc["total"];
         Serial.print("Total value in JSON: ");
         Serial.println(total);
     } else {
         Serial.print("Error HTTP: ");
         Serial.println(httpResponseCode);
         Serial.println("Treść odpowiedzi:");
         Serial.println(http.getString());
     }
     closeConnection(http);
}

void mright() {
   potValue = analogRead(potentiometerPin);
   while(potValue > 700) {
    potValue = analogRead(potentiometerPin);
     digitalWrite(SMXPin, LOW);
     digitalWrite(SMYPin, HIGH);
     Serial.println(potValue);
     delay(20);
     if (potValue < 700){
      break;
      }
     }
     digitalWrite(SMXPin, LOW);
     digitalWrite(SMYPin, LOW);
     Serial.println(potValue);
     Serial.println("STOP");
}

void mleft(){
   potValue = analogRead(potentiometerPin);
   while(potValue<1000) {
     potValue = analogRead(potentiometerPin);
     digitalWrite(SMXPin, HIGH);
     digitalWrite(SMYPin, LOW);
     Serial.println(potValue);
     delay(20);
     if (potValue > 1000){
      break;
      }
   }
     digitalWrite(SMXPin, LOW);
     digitalWrite(SMYPin, LOW);
     Serial.println(potValue);
     Serial.println("STOP");
}

void setup() {
     pinMode(potentiometerPin, INPUT);
     Serial.begin(115200);
     potValue = analogRead(potentiometerPin);
     pinMode(ledPin, OUTPUT);
     pinMode(RED_PIN, OUTPUT);
     pinMode(BLU_PIN, OUTPUT);
     pinMode(SMXPin, OUTPUT);
     pinMode(SMYPin, OUTPUT);
     pinMode(Buzzer, OUTPUT);
     delay(1000);
     digitalWrite(BLU_PIN, HIGH);
     delay(1000);
     Serial.println("LEFT");
     mleft();
     digitalWrite(BLU_PIN, LOW);
     digitalWrite(RED_PIN, HIGH);
     delay(1000);
     Serial.println("RIGHT");
     mright();
     digitalWrite(RED_PIN, LOW);

     digitalWrite(Buzzer, LOW);
     digitalWrite(RED_PIN, LOW);
     digitalWrite(BLU_PIN, HIGH);

     delay(1000);

     WiFi.disconnect(true);
     WiFi.mode(WIFI_STA);
     WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME,
EAP_PASSWORD);
     Serial.print("Łączenie z siecią WiFi");

     while (WiFi.status() != WL_CONNECTED) {
         digitalWrite(ledPin, HIGH);
         delay(500);
         digitalWrite(ledPin, LOW);
         delay(500);
         Serial.print(".");
         counter++;
         if (counter >= 60) { // 30 seconds timeout - reset board
             ESP.restart();
         }
     }

     Serial.println("");
     Serial.println("WiFi connected");
     Serial.println("Adress IP: " + WiFi.localIP().toString());
     //wait 30s stable connection
     delay(30000);
     //chceck connection
if (isInternetConnected()) {
         Serial.println("Testing connection pass");
         // chceck jira and light the led when total isnt 0
     } else {
         Serial.println("Testing connection fail");
         // in case of no wifi conection after 30s reset board
         ESP.restart();
     }
}

void loop() {
    pinMode(potentiometerPin, INPUT);
    potValue = analogRead(potentiometerPin);
    delay(10);
    Serial.println(potValue);
     if (total > 0) {
         mleft();
         digitalWrite(ledPin, LOW);
         delay(100);
         digitalWrite(ledPin, HIGH);
         digitalWrite(RED_PIN, HIGH);
         digitalWrite(BLU_PIN, LOW);
         Serial.println("LED_ON");
         if(playedOnce==true){
           digitalWrite(Buzzer, HIGH);
           delay(1000);
           digitalWrite(Buzzer, LOW);
           delay(500);
           digitalWrite(Buzzer, HIGH);
           delay(1000);
           digitalWrite(Buzzer, LOW);
           playedOnce=false;
         }
     }else{ 
         mright();
         digitalWrite(ledPin, HIGH);
         delay(100);
         digitalWrite(ledPin, LOW);
         Serial.println("LED_OFF");
         digitalWrite(RED_PIN, LOW);
         digitalWrite(BLU_PIN, HIGH);
         playedOnce=true;
     }
     delay(60000);
     checkJiraAndBlink(); // chceck jira and light the led when total isnt 0
}
