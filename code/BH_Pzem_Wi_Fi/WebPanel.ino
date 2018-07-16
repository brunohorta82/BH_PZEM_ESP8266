#include <ESP8266mDNS.h>

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

String lastReadings= "{}";

// SKETCH BEGIN
AsyncWebServer server(80);

void  prepareWebserver(){
  MDNS.begin(hostname.c_str());
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



