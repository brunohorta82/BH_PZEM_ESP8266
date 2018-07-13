#include <ArduinoJson.h>
#include <FS.h> 
#define EMPTY  ""
#define HARDWARE "bhpzem" 
#define FIRMWARE_VERSION 1.6
#define NODE_ID "mynode"
#define HOSTNAME String(HARDWARE)+"-"+String(NODE_ID)
#define MAX_ATTEMPTS 5
#define DELAY_NOTIFICATION 5000 //5 seconds
#define TEMPERATURE_PRECISION 9

#define WIFI_SSID "VOID SOFTWARE"
#define WIFI_SECRET "blackiscool"
//     ___ ___ ___ ___  _    
//    / __| _ \_ _/ _ \( )___
//   | (_ |  _/| | (_) |/(_-<
//    \___|_| |___\___/  /__/
//     
#define DIRECTION_PIN 14
#define RX_PIN 4
#define TX_PIN 5 
#define DS18B20_PIN 12


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
#define EMONCMS_PROTOCOL 0 //usar 0 para HTTP ou 1 HTTPS para servidores com certificado
#define EMONCMS_HOST  ""
#define EMONCMS_URL_PREFIX  ""
#define EMONCMS_API_KEY ""

//    __  __  ___ _____ _____ 
//   |  \/  |/ _ \_   _|_   _|
//   | |\/| | (_) || |   | |  
//   |_|  |_|\__\_\|_|   |_|  
//  
#define MQTT_BROKER_IP "192.168.187.227"
#define MQTT_BROKER_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""


//    ___ ___ ___ ___ _      ___   ___    
//   |   \_ _/ __| _ \ |    /_\ \ / ( )___
//   | |) | |\__ \  _/ |__ / _ \ V /|/(_-<
//   |___/___|___/_| |____/_/ \_\_|   /__/
//
//#define D_SSD1306
String hostname = HOSTNAME;
String nodeId = NODE_ID;
int relayCount = 2;
int notificationInterval = DELAY_NOTIFICATION;
bool directionCurrentDetection = DETECT_DIRECTION;
String emoncmsApiKey = EMONCMS_API_KEY;
String emoncmsUrl = EMONCMS_HOST;
String emoncmsPrefix = EMONCMS_URL_PREFIX;
String mqttIpDns = MQTT_BROKER_IP;
String mqttUsername = MQTT_USERNAME;
String mqttPassword = MQTT_PASSWORD;

bool emoncmshttp = EMONCMS_PROTOCOL ==  0;
String fileName = "/bconfig.json";
String wifiSSID = WIFI_SSID;
String wifiSecret = WIFI_SECRET;

const int totalAvailableGPIOs = 5;
String availableGPIOS[totalAvailableGPIOs]  ;
bool shouldReboot = false;
bool loadDefaults = false;
bool restartMqtt = false;
bool configNeedsUpdate = false;
String cachedConfigJson = "";
String nextConfigJson = "";

String buildConfigToJson(String _nodeId, int _notificationInterval, bool _directionCurrentDetection, String  _emoncmsApiKey, String _emoncmsPrefix, String  _emoncmsUrl, String _mqttIpDns, String _mqttUsername,String _mqttPassword ,String _wifiSSID, String _wifiSecret, String _IO_00, String  _IO_02, String _IO_13, String _IO_15, String _IO_16, String _hostname ){
          return "{\"nodeId\":\""+_nodeId+"\","+
          "\"hostname\":\""+String(_hostname)+"\","+
          "\"notificationInterval\":"+String(_notificationInterval)+","+
          "\"directionCurrentDetection\":"+String(_directionCurrentDetection)+","+
          "\"emoncmsApiKey\": \""+_emoncmsApiKey+"\","+
          "\"emoncmsPrefix\": \""+_emoncmsPrefix+"\","+
          "\"emoncmsUrl\": \""+_emoncmsUrl+"\","+
          "\"mqttIpDns\": \""+_mqttIpDns+"\","+
          "\"mqttUsername\": \""+_mqttUsername+"\","+
          "\"mqttPassword\": \""+_mqttPassword+"\","+
          "\"wifiSSID\": \""+_wifiSSID+"\","+
          "\"wifiSecret\": \""+_wifiSecret+"\","+
          "\"IO_16\": \""+_IO_16+"\","+
          "\"IO_13\": \""+_IO_13+"\","+
          "\"IO_00\": \""+_IO_00+"\","+
          "\"IO_02\": \""+_IO_02+"\","+
          "\"IO_15\": \""+_IO_15+"\""+
          "}";
}

String defaultConfigJson(){
   return buildConfigToJson(nodeId ,notificationInterval,directionCurrentDetection,emoncmsApiKey,emoncmsPrefix,emoncmsUrl,mqttIpDns,mqttUsername,mqttPassword,wifiSSID,wifiSecret,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,HOSTNAME);
}

void applyJsonConfig(String json) {
    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject &root = jsonBuffer.parseObject(json);
    if( !root.success()){
      Serial.println("[CONFIG] ERROR ON JSON VALIDATION!");
      return;
    }
    
    nodeId = root["nodeId"] | NODE_ID;
    hostname = String(HARDWARE) +"-"+String(nodeId);
    notificationInterval=root["notificationInterval"] | DELAY_NOTIFICATION;
    directionCurrentDetection=root["directionCurrentDetection"] | DETECT_DIRECTION;
    emoncmsApiKey=root["emoncmsApiKey"] | EMONCMS_API_KEY;
    emoncmsUrl=root["emoncmsUrl"] | EMONCMS_HOST;
    emoncmsPrefix=root["emoncmsPrefix"] | EMONCMS_URL_PREFIX;
    mqttIpDns=root["mqttIpDns"] | MQTT_BROKER_IP;
    mqttUsername = root["mqttUsername"] | MQTT_USERNAME;
    mqttPassword = root["mqttPassword"] | MQTT_PASSWORD;
    String lastSSID =  wifiSSID;
    String lastWifiSecrect =  wifiSecret;
    wifiSSID = root["wifiSSID"] | WIFI_SSID;
    wifiSecret = root["wifiSecret"] | WIFI_SECRET;
    availableGPIOS[0] = root["IO_16"] | "";
    availableGPIOS[1] =root["IO_13"] | "";
    availableGPIOS[2] = root["IO_00"] | "";
    availableGPIOS[3] = root["IO_02"] | "";
    availableGPIOS[4] =root["IO_15"] |"";
    if(wifiSSID != lastSSID ||  wifiSecret != lastWifiSecrect){
       jw.cleanNetworks();
       jw.addNetwork(wifiSSID.c_str(), wifiSecret.c_str());
    }
    cachedConfigJson = json ;
}


void loadStoredConfiguration(){
  String configJson = "";
  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(fileName)){
      cFile = SPIFFS.open(fileName,"r+"); 
     }else{
      cFile = SPIFFS.open(fileName,"w+"); 
      cFile.print(defaultConfigJson());
      }
     if(!cFile){
        Serial.println("[CONFIF] Create file config Error!");
      }else{
        Serial.println("[CONFIG] Read stored file config...");
        while(cFile.available()) {
          String line = cFile.readStringUntil('\n');
          configJson+= line;
        }
        cFile.close();
      }
  }else{
     Serial.println("Open file system Error!");
  }
   SPIFFS.end(); 
   applyJsonConfig(configJson);
}


bool checkRebootRules(String newConfig){
   DynamicJsonBuffer jsonBuffer(1024);
    JsonObject &root = jsonBuffer.parseObject(newConfig);
    if( !root.success()){
      Serial.println("[CONFIG] ERROR ON JSON VALIDATION!");
      return true;
    }    
   
  return nodeId != (root["nodeId"]  | NODE_ID) ||    hostname != String(HARDWARE) +"-"+String((root["nodeId"]  | NODE_ID)) || (WiFi.status() == WL_CONNECTED && (wifiSSID != (root["wifiSSID"] | WIFI_SSID) ||  wifiSecret != (root["wifiSecret"] | WIFI_SECRET)));
}

void updateServices(){
  restartMqtt = true; //TODO CHECK IF NEEDED
}

void saveConfig(String newConfig) {
   if(SPIFFS.begin()){
      File rFile = SPIFFS.open(fileName,"w+");
      if(!rFile){
        Serial.println("[CONFIG] Open config file Error!");
      } else {
        rFile.print(newConfig);
      }
      rFile.close();
   }else{
     Serial.println("[CONFIG] Open file system Error!");
  }
  SPIFFS.end();
  configNeedsUpdate = false;
  
  Serial.println("[CONFIG] New config loaded.");
  shouldReboot = checkRebootRules(newConfig);
  if(!shouldReboot){
   applyJsonConfig(nextConfigJson);
   updateServices();
   nextConfigJson = "";
  }
}




void requestToSaveNewConfigJson(String newConfig){
  if(newConfig.equals("")){
     Serial.println("[CONFIG] Invalid Config File");
     return;
   }
   cachedConfigJson = newConfig;
   nextConfigJson = newConfig;
   configNeedsUpdate = true;
}

void checkConfigChanges(){
  if(configNeedsUpdate && !nextConfigJson.equals("")){
    saveConfig(nextConfigJson);
   }
}


