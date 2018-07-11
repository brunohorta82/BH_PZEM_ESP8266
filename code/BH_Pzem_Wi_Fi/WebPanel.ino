
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <FS.h> 
ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/firmware";
String cachedConfigJson = "";
String lastReadings= "{}";
void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " +server.argName(i) + ": " +server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  
}

String configToJson(){
  cachedConfigJson =  "{\"nodeId\":\""+nodeId+"\","+
          "\"notificationInterval\":"+notificationInterval+","+
          "\"directionCurrentDetection\":"+directionCurrentDetection+","+
          "\"firmwareVersion\":"+firmwareVersion+","+
          "\"emoncmsApiKey\": \""+emoncmsApiKey+"\","+
          "\"emoncmsPrefix\": \""+emoncmsPrefix+"\","+
          "\"emoncmsUrl\": \""+emoncmsUrl+"\","+
          "\"mqttIpDns\": \""+mqttIpDns+"\","+
          "\"mqttUsername\": \""+mqttUsername+"\","+
          "\"mqttPassword\": \""+mqttPassword+"\""+
          "}";
  return  cachedConfigJson;
}

void saveConfig() {
   if(SPIFFS.begin()){
      File rFile = SPIFFS.open(fileName,"w+");
      if(!rFile){
        Serial.println("Open config file Error!");
      } else {
        rFile.print(configToJson());
      }
      rFile.close();
   }else{
     Serial.println("Open file system Error!");
  }
  SPIFFS.end();
}



String loadConfiguration(){
  if(!cachedConfigJson.equals("")){
    return cachedConfigJson;
   }

  if(SPIFFS.begin()){
    File cFile;   
    if(SPIFFS.exists(fileName)){
    cFile = SPIFFS.open(fileName,"r+"); 
     }else{
      cFile = SPIFFS.open(fileName,"w+"); 
      cFile.print(configToJson());
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
 
void handleJQuery() {
  server.sendHeader("content-encoding","gzip");
  server.sendHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
  server.send_P(200, "application/js",jQuery,sizeof(jQuery));
}
void handleCss() {
  server.sendHeader("content-encoding","gzip");
  server.sendHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
  server.send_P(200, "text/css",css,sizeof(css));
}
void handleJs() {
  server.send_P(200, "application/js",js);
}
void handleRoot() {
  server.send_P(200, "text/html", index_html);
}
void handleReadings(){
  server.send(200, "application/json",lastReadings);
}
void handleSaveConfig() {
    nodeId = server.arg("nodeId");
    Serial.println(nodeId);
    notificationInterval=server.arg("notificationInterval").toInt();
    Serial.println( notificationInterval);
    directionCurrentDetection= server.hasArg("directionCurrentDetection");
    Serial.println( directionCurrentDetection);
    emoncmsApiKey=server.arg("emoncmsApiKey");
    Serial.println(   emoncmsApiKey);
    emoncmsUrl=server.arg("emoncmsUrl");
    emoncmsPrefix=server.arg("emoncmsPrefix");
    mqttIpDns=server.arg("mqttIpDns");
    mqttUsername=server.arg("mqttUsername");
    mqttPassword=server.arg("mqttPassword");
    Serial.println(configToJson());
    saveConfig();
    server.sendHeader("Location", String("/"), true);
    server.send ( 302);
}
void loadLastConfig(String json) {
    StaticJsonBuffer<512> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(json);
    nodeId = root["nodeId"] | NODE_ID;
    notificationInterval=root["notificationInterval"] | DELAY_NOTIFICATION;
    directionCurrentDetection=root["directionCurrentDetection"] | DETECT_DIRECTION;
    emoncmsApiKey=root["emoncmsApiKey"] | EMONCMS_API_KEY;
    emoncmsUrl=root["emoncmsUrl"] | EMONCMS_HOST;
    emoncmsPrefix=root["emoncmsPrefix"] | EMONCMS_URL_PREFIX;
    mqttIpDns=root["mqttIpDns"] | MQTT_BROKER_IP;
    mqttUsername = root["mqttUsername"] | MQTT_USERNAME;
    mqttPassword = root["mqttPassword"] | MQTT_PASSWORD;
}
void handleConfig() {
  server.send(200, "application/json",loadConfiguration());
}

void prepareWebserver(){
  MDNS.begin(HOSTNAME);
  httpUpdater.setup(&server, update_path);
  MDNS.addService("http", "tcp", 80);
  loadConfiguration();
  server.on("/", handleRoot);
  server.on("/jquery.min.js", handleJQuery);
  server.on("/js-index.js", handleJs);
  server.on("/bootstrap.css", handleCss);
  server.on("/readings", handleReadings);
  server.on("/config", handleConfig);
  server.on("/saveconfig", handleSaveConfig);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.printf("BH PZEM - Configuration Panel ready! Open http://%s.local in your browser'\n", HOSTNAME);
}
void webServerLoop(){
 server.handleClient();
}
void publishOnPanel(String json){
lastReadings = json;
}
