#define FIRMWARE_VERSION 4.4

//#define PZEM004
#define PZEMDC
#define FACTORY_TYPE "pzem017" //pzem004T pzem017 
#define HARDWARE "bhpzem"

#include <ESP8266httpUpdate.h>
#include <JustWifi.h> //https://github.com/xoseperez/justwifi
#include <ESP8266mDNS.h>
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <Timing.h> //https://github.com/scargill/Timing
#include <AsyncMqttClient.h> //https://github.com/marvinroger/async-mqtt-client
#include <ArduinoJson.h> //Install from Arduino IDE Library Manager
#include "FS.h"
#include <Ticker.h>
#include <ESPAsyncTCP.h> //https://github.com/me-no-dev/ESPAsyncTCP
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer
#include <AsyncJson.h> //https://github.com/me-no-dev/ESPAsyncWebServer

#define CONFIG_FILENAME  "/config_"+String(HARDWARE)+".json"
#define CONFIG_BUFFER_SIZE 1024

//WIFI
#define WIFI_SSID ""
#define WIFI_SECRET ""

//AP PASSWORD  
#define AP_SECRET "EasyIot@"


//MQTT  
#define MQTT_BROKER_IP ""
#define MQTT_BROKER_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""


//CONTROL FLAGS
bool shouldReboot = false;
bool reloadMqttConfiguration = false;
bool wifiUpdated = false;
bool laodDefaults = false;
bool adopted = false;
bool autoUpdate = false;
int easyConfig = 0;
DynamicJsonBuffer jsonBuffer(CONFIG_BUFFER_SIZE);

JsonArray& getJsonArray() {
    return jsonBuffer.createArray();
}

JsonArray& getJsonArray(File file) {
    return jsonBuffer.parseArray(file);
}

JsonObject& getJsonObject() {
    return jsonBuffer.createObject();
}

JsonObject& getJsonObject(File file) {
    return jsonBuffer.parseObject(file);
}

JsonObject& getJsonObject(const char* data) {
    return jsonBuffer.parseObject(data);
}
