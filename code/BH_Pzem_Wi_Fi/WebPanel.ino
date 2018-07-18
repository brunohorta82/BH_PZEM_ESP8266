#include "static_site.h"
#include "static_css.h"
#include "static_js.h"
#include "static_fonts.h"
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h> //https://github.com/me-no-dev/ESPAsyncTCP
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer

String lastReadings= "{}";

// SKETCH BEGIN
AsyncWebServer server(80);

void  prepareWebserver(){
  MDNS.begin(hostname.c_str());
  MDNS.addService("http","tcp",80);
  /** HTML  **/
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html,sizeof(index_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/dashboard.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", dashboard_html,sizeof(dashboard_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/emoncms.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", emoncms_html,sizeof(emoncms_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/firmware.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", firmware_html,sizeof(firmware_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/gpios.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", gpios_html,sizeof(gpios_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/mqtt.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", mqtt_html,sizeof(mqtt_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/node.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", node_html,sizeof(node_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/wifi.html", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", wifi_html,sizeof(wifi_html));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  /** JS    **/
  server.on("/js/adminlte.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/js", adminlte_min_js,sizeof(adminlte_min_js));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });
  server.on("/js/GaugeMeter.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/js", GaugeMeter_js,sizeof(GaugeMeter_js));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });
  server.on("/js/index.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/js", index_js,sizeof(index_js));
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  server.on("/js/jquery.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(200, "application/js", jquery_min_js,sizeof(jquery_min_js));
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Expires","Mon, 1 Jan 2222 10:10:10 GMT");
    request->send(response);
  });
   
  /** CSS   **/
  /** FONTS **/

  /** JSON **/ 
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", lastReadings);
  });
 server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200,  "application/json","["+cachedConfigJson+",{\"firmwareVersion\":"+String(FIRMWARE_VERSION)+"}]");
  });
  /** POSTS **/
   server.on("/saveconfig", HTTP_POST, [](AsyncWebServerRequest *request){
   String newConfig=buildConfigToJson(
    request->arg("nodeId"),
   request->arg("notificationInterval").toInt(),
   request->hasArg("directionCurrentDetection"),
   request->arg("emoncmsApiKey"),
   request->arg("emoncmsPrefix"),
   request->arg("emoncmsUrl"),
   request->arg("mqttIpDns"),
   request->arg("mqttUsername"),
   request->arg("mqttPassword"),
   request->arg("wifiSSID"),
   request->arg("wifiSecret"),
   request->arg("IO_00"),
   request->arg("IO_02"),
   request->arg("IO_13"),
   request->arg("IO_15"),
   request->arg("IO_16"),
   hostname);
   requestToSaveNewConfigJson(newConfig);
   request->redirect("/");
  });
  
   server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request){
    shouldReboot = true;
   request->send(200);
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

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
}


void publishOnPanel(String json){
  lastReadings = json;
}



