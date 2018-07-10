
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/firmware";
const char index_html[] PROGMEM={};


void prepareWebserver(){
  MDNS.begin(HOSTNAME);
  httpUpdater.setup(&httpServer, update_path);

  MDNS.addService("http", "tcp", 80);
  httpServer.on("/", handleRoot);
  httpServer.onNotFound(handleNotFound);
  httpServer.begin();
  Serial.printf("BH PZEM - Configuration Panel ready! Open http://%s.local%s in your browser'\n", HOSTNAME, update_path);
}

void webServerLoop(){
  httpServer.handleClient();
}
void handleRoot() {
 
  httpServer.send_P(200, "text/html", index_html);
 
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";
  for (uint8_t i=0; i<httpServer.args(); i++){
    message += " " +httpServer.argName(i) + ": " +httpServer.arg(i) + "\n";
  }
  httpServer.send(404, "text/plain", message);
  
}

