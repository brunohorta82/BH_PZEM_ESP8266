#include <ESP8266mDNS.h>
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
  MDNS.begin(String(HOSTNAME).c_str());
  MDNS.addService("http","tcp",80);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html",index_html);
  });
 server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/css",style_css);
  });

  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", lastReadings);
  });
 server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200,  "application/json","["+cachedConfigJson+",{\"firmwareVersion\":"+String(FIRMWARE_VERSION)+"}]");
  });

   server.on("/saveconfig", HTTP_POST, [](AsyncWebServerRequest *request){
   cachedConfigJson= "{\"nodeId\":\""+request->arg("nodeId")+"\","+
          "\"notificationInterval\":"+String(request->arg("notificationInterval").toInt())+","+
          "\"directionCurrentDetection\":"+request->hasArg("directionCurrentDetection")+","+
          "\"emoncmsApiKey\": \""+request->arg("emoncmsApiKey")+"\","+
          "\"emoncmsPrefix\": \""+request->arg("emoncmsPrefix")+"\","+
          "\"emoncmsUrl\": \""+request->arg("emoncmsUrl")+"\","+
          "\"mqttIpDns\": \""+request->arg("mqttIpDns")+"\","+
          "\"mqttUsername\": \""+request->arg("mqttUsername")+"\","+
          "\"mqttPassword\": \""+request->arg("mqttPassword")+"\","+
          "\"wifiSSID\": \""+request->arg("wifiSSID")+"\","+
          "\"wifiSecret\": \""+request->arg("wifiSecret")+"\","+
          "\"IO_16\": \""+request->arg("IO_16")+"\","+
          "\"IO_13\": \""+request->arg("IO_13")+"\","+
          "\"IO_00\": \""+request->arg("IO_00")+"\","+
          "\"IO_02\": \""+request->arg("IO_02")+"\","+
          "\"IO_15\": \""+request->arg("IO_15")+"\""+
          "}";
    saveConfig();
    request->redirect("/");
    configChanged = true;
  });
  server.on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "application/js",index_js);
  });
  server.on("/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/js", jQuery,sizeof(jQuery));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });
  server.on("/GaugeMeter.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/js", gauge,sizeof(gauge));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });
   server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/css", css,sizeof(css));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });


  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request){
    shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot?"OK":"FAIL");
    response->addHeader("Connection", "close");
    request->send(response);
  },[](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
    if(!index){
      Serial.printf("Update Start: %s\n", filename.c_str());
      Update.runAsync(true);
      if(!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)){
        Update.printError(Serial);
      }
    }
    if(!Update.hasError()){
      if(Update.write(data, len) != len){
        Update.printError(Serial);
      }
    }
    if(final){
      if(Update.end(true)){
        Serial.printf("Update Success: %uB\n", index+len);
      } else {
        Update.printError(Serial);
      }
    }
  });

server.onNotFound([](AsyncWebServerRequest *request) {
  if (request->method() == HTTP_OPTIONS) {
    request->send(200);
  } else {
    request->send(404);
  }
});
 server.onNotFound([](AsyncWebServerRequest *request){  
  if(request->method() == HTTP_OPTIONS)
     request->send(200);
    else
    request->send(404);
});

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
}
void publishOnPanel(String json){
lastReadings = json;
}
String loadConfiguration(){
  if(!cachedConfigJson.equals("")){
    cachedConfigJson;
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
    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject &root = jsonBuffer.parseObject(json);
    int cv = root["CONFIG_VERSION"] | -1;
    if( !root.success() || loadDefaults ){
       loadDefaults  = false;
      Serial.println("[CONFIG] New Version of config has detected");
     cachedConfigJson= "{\"nodeId\":\""+String(nodeId)+"\","+
          "\"notificationInterval\":"+String(notificationInterval)+","+
          "\"directionCurrentDetection\":"+String(directionCurrentDetection)+","+
          "\"emoncmsApiKey\": \""+emoncmsApiKey+"\","+
          "\"emoncmsPrefix\": \""+emoncmsPrefix+"\","+
          "\"emoncmsUrl\": \""+emoncmsUrl+"\","+
          "\"mqttIpDns\": \""+mqttIpDns+"\","+
          "\"mqttUsername\": \""+mqttUsername+"\","+
          "\"mqttPassword\": \""+mqttPassword+"\","+
          "\"wifiSSID\": \""+wifiSSID+"\","+
          "\"wifiSecret\": \""+wifiSecret+"\","+
          "\"IO_16\": \""+availableGPIOS[0]+"\","+
          "\"IO_13\": \""+availableGPIOS[1]+"\","+
          "\"IO_00\": \""+availableGPIOS[2]+"\","+
          "\"IO_02\": \""+availableGPIOS[3]+"\","+
          "\"IO_15\": \""+availableGPIOS[4]+"\""+
          "}";
          saveConfig();
          configChanged = true;
           shouldReboot = true;
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
      shouldReboot = true;
    }
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
   shouldReboot = true;
    }
 }
}

