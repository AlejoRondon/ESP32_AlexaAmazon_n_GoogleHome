#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <StreamString.h>

#define API_KEY "a7256f1b-797c-46a0-8641-bc1eea905e88" // TODO: Change to your sinric API Key. Your API Key is displayed on sinric.com dashboard
#define SSID_NAME "alegria" // TODO: Change to your Wifi network SSID
#define WIFI_PASSWORD "pensamiento" // TODO: Change to your Wifi network password
#define SERVER_URL "iot.sinric.com"
#define SERVER_PORT 80 

#define HEARTBEAT_INTERVAL 300000 // 5 Minutes 

WiFiMulti WiFiMultiESP32;
WebSocketsClient webSocket;
WiFiClient client;

uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

void setPowerStateOnServer(String deviceId, String value);
void setTargetTemperatureOnServer(String deviceId, String value, String scale);
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

void setup() {
  Serial.begin(115200);
  
  WiFiMultiESP32.addAP(SSID_NAME, WIFI_PASSWORD);
  Serial.println();
  Serial.print("Connecting to Wifi: ");
  Serial.println(SSID_NAME);  

  // Waiting for Wifi connect
  while(WiFiMultiESP32.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  if(WiFiMultiESP32.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.print("WiFi connected. ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }

  // server address, port and URL
  webSocket.begin(SERVER_URL, SERVER_PORT, "/");

  // event handler
  webSocket.onEvent(webSocketEvent);
  webSocket.setAuthorization("apikey", API_KEY);
  
  // try again every 5000ms if connection has failed
  webSocket.setReconnectInterval(5000);   // If you see 'class WebSocketsClient' has no member named 'setReconnectInterval' error update arduinoWebSockets
}

void loop() {
  webSocket.loop();
  
  if(isConnected) {
      uint64_t now = millis();
      
      // Send heartbeat in order to avoid disconnections during ISP resetting IPs over night. Thanks @MacSass
      if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
          heartbeatTimestamp = now;
          webSocket.sendTXT("H");          
      }
  }   
}
  
void lockDoor(String deviceId) {
  if (deviceId == "5f197d1ead7a48327f37649c") // Device ID of first device
  {  
    Serial.print("Locking device id: ");
    Serial.println(deviceId);
  }     
}

void unlockDoor(String deviceId) {
   if (deviceId == "5f197d1ead7a48327f37649c") // Device ID of first device
   {  
     Serial.print("Unlocking off Device ID: ");
     Serial.println(deviceId);
   }
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      isConnected = false;    
      Serial.printf("[webSocketEvent] Webservice disconnected from server!\n");
      break;
    case WStype_CONNECTED: {
      isConnected = true;
      Serial.printf("[webSocketEvent] Service connected to server at url: %s\n", payload);
      Serial.printf("[webSocketEvent] Waiting for commands from server ...\n");        
      }
      break;
    case WStype_TEXT: {
        Serial.printf("[webSocketEvent] get text: %s\n", payload);
#if ARDUINOJSON_VERSION_MAJOR == 5
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject((char*)payload);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
        DynamicJsonDocument json(1024);
        deserializeJson(json, (char*) payload);      
#endif        
        String deviceId = json ["deviceId"];     
        String action = json ["action"];
        
        if(action == "Unlock") {
            // alexa, turn on tv ==> {"deviceId":"xx","action":"setPowerState","value":"ON"}
//            String value = json ["value"];
//            if(value == "ON") {
//                turnOn(deviceId);
//            } else {
//                turnOff(deviceId);
//            }     
              unlockDoor(deviceId) ;  
        }
        else if(action == "Lock") {
          lockDoor(deviceId);
        }
        else if(action == "AdjustBrightness") {
            // alexa, dim lights  ==>{"deviceId":"xxx","action":"AdjustBrightness","value":-25}
        }       
        else if(action == "AdjustBrightness") {
            // alexa, dim lights  ==>{"deviceId":"xx","action":"AdjustBrightness","value":-25}
        }
        else if(action == "SetBrightness") {
           //alexa, set the lights to 50% ==> {"deviceId":"xx","action":"SetBrightness","value":50}
        }
        else if(action == "SetColor") {
           //alexa, set the lights to red ==> {"deviceId":"xx","action":"SetColor","value":{"hue":0,"saturation":1,"brightness":1}}
        }
        else if(action == "IncreaseColorTemperature") {
           //alexa, set the lights softer ==> {"deviceId":"xxx","action":"IncreaseColorTemperature"}
        }
        else if(action == "IncreaseColorTemperature") {
           //alexa, set the lights softer ==> {"deviceId":"xxx","action":"IncreaseColorTemperature"}
        }
        else if(action == "SetColorTemperature") {
           //alexa, set the lights softer ==> {"deviceId":"xxx","action":"SetColorTemperature","value":2200}
        }
      }
      break;
    case WStype_BIN:
      Serial.printf("[webSocketEvent] get binary length: %u\n", length);
      break;
    default: break;
  }
}


// If you are going to use a push button to on/off the switch manually, use this function to update the status on the server
// so it will reflect on Alexa app.
// eg: setPowerStateOnServer("deviceid", "ON")

// Call ONLY If status changed. DO NOT CALL THIS IN loop() and overload the server. 
void setPowerStateOnServer(String deviceId, String value) {
#if ARDUINOJSON_VERSION_MAJOR == 5
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
  DynamicJsonDocument root(1024);
#endif        
  root["deviceId"] = deviceId;
  root["action"] = "unlock";
  root["value"] = value;
  StreamString databuf;
#if ARDUINOJSON_VERSION_MAJOR == 5
  root.printTo(databuf);
#endif
#if ARDUINOJSON_VERSION_MAJOR == 6        
  serializeJson(root, databuf);
#endif  
  
  webSocket.sendTXT(databuf);
}
