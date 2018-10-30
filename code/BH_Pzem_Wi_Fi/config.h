#include <JustWifi.h> //https://github.com/xoseperez/justwifi
#include <ESP8266mDNS.h>
#include <Timing.h> //https://github.com/scargill/Timing
#include <AsyncMqttClient.h> //https://github.com/marvinroger/async-mqtt-client
#include <DallasTemperature.h> //https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <ArduinoJson.h> ////Install from Arduino IDE Library Manager
#include <FS.h> 
#include <Ticker.h>
#include <ESPAsyncTCP.h> //https://github.com/me-no-dev/ESPAsyncTCP
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer
#define HARDWARE "bhpzem" 
#define FIRMWARE_VERSION 2.4
#define NODE_ID "mynode"
#define HOSTNAME String(HARDWARE)+"-"+String(NODE_ID)
#define CONFIG_FILENAME  "/config_"+String(HARDWARE)+".json"
#define MAX_ATTEMPTS 5
#define DELAY_NOTIFICATION 5000 //5 seconds
#define TEMPERATURE_PRECISION 9
#define CONFIG_BUFFER_SIZE 1024
#define WIFI_SSID ""
#define WIFI_SECRET ""
//     ___ ___ ___ ___  _    
//    / __| _ \_ _/ _ \( )___
//   | (_ |  _/| | (_) |/(_-<
//    \___|_| |___\___/  /__/
//     
#define DIRECTION_PIN 14
#define RX_PIN 4
#define TX_PIN 5 
#define DS18B20_PIN 12
#define DISPLAY_SDA 2 //-1 if you don't use display
#define DISPLAY_SCL 13 //-1 if you don't use display
#define DISPLAY_BTN 16

//    ___ ___ ___ _____ _   _ ___ ___ ___ 
//   | __| __/ __|_   _| | | | _ \ __/ __|
//   | _|| _| (__  | | | |_| |   / _|\__ \
//   |_| |___\___| |_|  \___/|_|_\___|___/
// 
#define DETECT_DIRECTION false
#define PRINT_TO_SERIAL_MONITOR false

//    ___  ___  ___ _____ _    
//   | _ \/ _ \| _ \_   _( )___
//   |  _/ (_) |   / | | |/(_-<
//   |_|  \___/|_|_\ |_|   /__/
//   
#define HTTPS_PORT  443
#define HTTP_PORT  80
//    ___ __  __  ___  _  _  ___ _  _ ___ 
//   | __|  \/  |/ _ \| \| |/ __| \| / __|
//   | _|| |\/| | (_) | .` | (__| .` \__ \
//   |___|_|  |_|\___/|_|\_|\___|_|\_|___/
//   
#define EMONCMS_HOST  ""
#define EMONCMS_URL_PREFIX  ""
#define EMONCMS_API_KEY ""
#define EMONCMS_PORT 80

//    __  __  ___ _____ _____ 
//   |  \/  |/ _ \_   _|_   _|
//   | |\/| | (_) || |   | |  
//   |_|  |_|\__\_\|_|   |_|  
//  
#define MQTT_BROKER_IP ""
#define MQTT_BROKER_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define PAYLOAD_ON "ON"
#define PAYLOAD_OFF "OFF"
#define PAYLOAD_PULSE_OFF_ON "PULSE_OFF"
#define PAYLOAD_PULSE_ON_OFF "PULSE_ON"

//NODE
String hostname = HOSTNAME;
String nodeId = NODE_ID;
bool directionCurrentDetection = DETECT_DIRECTION;
String cachedReadings = "{}";
//RELAYS
int notificationInterval = DELAY_NOTIFICATION;


//EMONCMS
String emoncmsApiKey = EMONCMS_API_KEY;
String emoncmsUrl = EMONCMS_HOST;
String emoncmsPrefix = EMONCMS_URL_PREFIX;
int emoncmsPort = EMONCMS_PORT;

//MQTT
String mqttIpDns = MQTT_BROKER_IP;
String mqttUsername = MQTT_USERNAME;
String mqttPassword = MQTT_PASSWORD;
String baseTopic = String(HARDWARE)+"/"+nodeId;
String availableTopic = String(HARDWARE)+"_"+nodeId+"/status";

//WI-FI
String wifiSSID = WIFI_SSID;
String wifiSecret = WIFI_SECRET;

//CONTROL FLAGS
bool configNeedsUpdate = false;
bool restartMqtt = false;
bool shouldReboot = false;
