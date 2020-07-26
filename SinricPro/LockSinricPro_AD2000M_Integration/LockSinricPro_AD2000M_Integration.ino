/*
 * Example for smart lock
 * 
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 *   - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
       #define DEBUG_ESP_PORT Serial
       #define NODEBUG_WEBSOCKETS
       #define NDEBUG
#endif 

#include <Arduino.h>
#ifdef ESP8266 
       #include <ESP8266WiFi.h>
#endif 
#ifdef ESP32   
       #include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProLock.h"

#define WIFI_SSID         "Familia Rondon"    
#define WIFI_PASS         "emilia1964"
#define APP_KEY           "8f931a1b-6e73-4dd5-ba12-2438519e1874"      // Should look like "de0bxxxx-1x3x-4x3x-ax2x-5dabxxxxxxxx"
#define APP_SECRET        "fef85bc4-06a0-49d7-ae52-2f0393b72f2d-f9bb8ca5-9b75-4c1a-93c2-f59bd385d37c"   // Should look like "5f36xxxx-x3x7-4x3x-xexe-e86724a9xxxx-4c4axxxx-3x3x-x5xe-x9x3-333d65xxxxxx"
#define LOCK_ID          "5f1c6c59e4c7460fd5b3a1e2"    // Should look like "5dc1564130xxxxxxxxxxxxxx"
#define BAUD_RATE         115200                // Change baudrate to your need
#define PIN_LED           1
#define PIN_RELAY         0

#define OPEN_DOOR         digitalWrite(PIN_RELAY,LOW)
#define CLOSE_DOOR        digitalWrite(PIN_RELAY,HIGH)
SinricProLock &myLock = SinricPro[LOCK_ID];
bool doorWasOpened = false;


// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 50;           // interval at which to blink (milliseconds)
int interval_counter = 0;
bool onLockState(String deviceId, bool &lockState) {
  Serial.printf("Device %s is %s\r\n", deviceId.c_str(), lockState?"locked":"unlocked");
  if(!lockState){
    OPEN_DOOR;
    doorWasOpened = true;
    previousMillis = millis();
  }
  return true;
}

void setupWiFi() {
  Serial.printf("\r\n[Wifi]: Connecting");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  IPAddress localIP = WiFi.localIP();
  Serial.printf("connected!\r\n[WiFi]: IP-Address is %d.%d.%d.%d\r\n", localIP[0], localIP[1], localIP[2], localIP[3]);
}


void setupSinricPro() {
  myLock.onLockState(onLockState);

  // setup SinricPro
  SinricPro.onConnected([](){ Serial.printf("Connected to SinricPro\r\n"); }); 
  SinricPro.onDisconnected([](){ Serial.printf("Disconnected from SinricPro\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
}


void setup() {
  Serial.begin(BAUD_RATE); Serial.printf("\r\n\r\n");
    // set the digital pin as output:
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_RELAY, HIGH);
  
  setupWiFi();
  setupSinricPro();
}

void loop() {
  SinricPro.handle();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    if (doorWasOpened) {
          CLOSE_DOOR;
      if(interval_counter++ == 40){
          interval_counter = 0;
          doorWasOpened = false;
          myLock.sendLockStateEvent(true, "PHYSYCAL_INTERACTION");
      }
    }

    // set the LED with the ledState of the variable:
    digitalWrite(PIN_LED, !digitalRead(PIN_LED));
   
  }
}
