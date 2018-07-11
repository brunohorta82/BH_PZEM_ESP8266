
#include <ArduinoJson.h>
#include <FS.h> 
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

String cachedConfigJson = "";
String lastReadings= "{}";
bool configChanged = false;
// SKETCH BEGIN
AsyncWebServer server(80);

void  prepareWebserver(){
  //MDNS.addService("http","tcp",80);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html",index_html);
  });

  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json",lastReadings);
  });
 server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200,  "application/json",loadConfiguration());
  });

   server.on("/saveconfig", HTTP_POST, [](AsyncWebServerRequest *request){
   cachedConfigJson= "{\"nodeId\":\""+request->arg("nodeId")+"\","+
          "\"notificationInterval\":"+String(request->arg("notificationInterval").toInt())+","+
          "\"directionCurrentDetection\":"+request->hasArg("directionCurrentDetection")+","+
          "\"firmwareVersion\":"+firmwareVersion+","+
          "\"emoncmsApiKey\": \""+request->arg("emoncmsApiKey")+"\","+
          "\"CONFIG_VERSION\": "+String(CONFIG_VERSION)+","+
          "\"emoncmsPrefix\": \""+request->arg("emoncmsPrefix")+"\","+
          "\"emoncmsUrl\": \""+request->arg("emoncmsUrl")+"\","+
          "\"mqttIpDns\": \""+request->arg("mqttIpDns")+"\","+
          "\"mqttUsername\": \""+request->arg("mqttUsername")+"\","+
          "\"mqttPassword\": \""+request->arg("mqttPassword")+"\","+
          "\"wifiSSID\": \""+request->arg("wifiSSID")+"\","+
          "\"wifiSecret\": \""+request->arg("wifiSecret")+"\""+
          "}";
    saveConfig();
    request->redirect("/");
    configChanged = true;
  });
  server.on("/js-index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/js",js);
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/js", jQuery,sizeof(jQuery));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });
  
   server.on("/bootstrap.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", css,sizeof(css));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });
  server.begin();
}
void publishOnPanel(String json){
lastReadings = json;
}
String loadConfiguration(){
  if(!cachedConfigJson.equals("")){
    Serial.println(cachedConfigJson);
    return cachedConfigJson;
   }

  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(fileName)){
    cFile = SPIFFS.open(fileName,"r+"); 
     }else{
      cFile = SPIFFS.open(fileName,"w+"); 
      cFile.print(cachedConfigJson);
      }
     if(!cFile){
        Serial.println("Create file config Error!");
      }else{
        Serial.println("Read Config");
        while(cFile.available()) {
          String line = cFile.readStringUntil('\n');
          cachedConfigJson+= line;
          loadLastConfig(cachedConfigJson);
        }
        cFile.close();
      }
  }else{
     Serial.println("Open file system Error!");
  }
   SPIFFS.end();
   return cachedConfigJson;
}
void loadLastConfig(String json) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(json);
    int cv = root["CONFIG_VERSION"] | -1;
    if(cv != CONFIG_VERSION){
      Serial.println("[CONFIG] New Version of config has detected");
     cachedConfigJson= "{\"nodeId\":\""+String(nodeId)+"\","+
          "\"notificationInterval\":"+String(notificationInterval)+","+
          "\"directionCurrentDetection\":"+String(directionCurrentDetection)+","+
          "\"firmwareVersion\":"+firmwareVersion+","+
          "\"emoncmsApiKey\": \""+emoncmsApiKey+"\","+
          "\"CONFIG_VERSION\": "+String(CONFIG_VERSION)+","+
          "\"emoncmsPrefix\": \""+emoncmsPrefix+"\","+
          "\"emoncmsUrl\": \""+emoncmsUrl+"\","+
          "\"mqttIpDns\": \""+mqttIpDns+"\","+
          "\"mqttUsername\": \""+mqttUsername+"\","+
          "\"mqttPassword\": \""+mqttPassword+"\","+
          "\"wifiSSID\": \""+wifiSSID+"\","+
          "\"wifiSecret\": \""+wifiSecret+"\""+
          "}";
          saveConfig();
          configChanged = true;
          ESP.restart();
          return;
    }
    nodeId = root["nodeId"] | NODE_ID;
    notificationInterval=root["notificationInterval"] | DELAY_NOTIFICATION;
    directionCurrentDetection=root["directionCurrentDetection"] | DETECT_DIRECTION;
    emoncmsApiKey=root["emoncmsApiKey"] | EMONCMS_API_KEY;
    emoncmsUrl=root["emoncmsUrl"] | EMONCMS_HOST;
    emoncmsPrefix=root["emoncmsPrefix"] | EMONCMS_URL_PREFIX;
    mqttIpDns=root["mqttIpDns"] | MQTT_BROKER_IP;
    mqttUsername = root["mqttUsername"] | MQTT_USERNAME;
    mqttPassword = root["mqttPassword"] | MQTT_PASSWORD;
    wifiSSID = root["wifiSSID"] | WIFI_SSID;
    wifiSecret = root["wifiSecret"] | WIFI_SECRET;
}

void saveConfig() {
   if(SPIFFS.begin()){
      File rFile = SPIFFS.open(fileName,"w+");
      if(!rFile){
        Serial.println("Open config file Error!");
      } else {
        rFile.print(cachedConfigJson);
      }
      rFile.close();
   }else{
     Serial.println("Open file system Error!");
  }
  SPIFFS.end();

}

void loadNewConfig(){
if(configChanged){
  Serial.println("[CONFIG] New config loaded.");
  loadLastConfig(cachedConfigJson);
  setupMQTT();
  configChanged = false;
  if(!WiFi.isConnected()){
    ESP.restart();
    }
 }
}

