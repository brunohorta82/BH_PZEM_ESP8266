
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char* update_path = "/firmware";
const char index_html[] PROGMEM={"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"    <link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.1/css/bootstrap.min.css\" integrity=\"sha384-WskhaSGFgHYWDcbwN70/dfYBj47jz9qbsMId/iRN3ewGhXQFZCSftd1LZCfmhktB\" crossorigin=\"anonymous\">\n"
"\n"
"</head>\n"
"<style>\n"
"    input[type=text], select {\n"
"        width: 100%;\n"
"        padding: 12px 20px;\n"
"        margin: 8px 0;\n"
"        display: inline-block;\n"
"        border: 1px solid #ccc;\n"
"        border-radius: 4px;\n"
"        box-sizing: border-box;\n"
"    }\n"
"    input[type=password], select {\n"
"        width: 100%;\n"
"        padding: 12px 20px;\n"
"        margin: 8px 0;\n"
"        display: inline-block;\n"
"        border: 1px solid #ccc;\n"
"        border-radius: 4px;\n"
"        box-sizing: border-box;\n"
"    }\n"
"    input[type=submit] {\n"
"        width: 100%;\n"
"        background-color: #08c;\n"
"        color: white;\n"
"        padding: 14px 20px;\n"
"        margin: 8px 0;\n"
"        border: none;\n"
"        text-transform: uppercase;\n"
"        font-weight: bold;\n"
"        border-radius: 4px;\n"
"        cursor: pointer;\n"
"    }\n"
"\n"
"    input[type=submit]:hover {\n"
"        background-color: #005580;\n"
"\n"
"    }\n"
"\n"
"    .box-config {\n"
"        border-radius: 5px;\n"
"        background-color: #333;\n"
"        padding: 20px;\n"
"    }\n"
"</style>\n"
"<body style=\"background-color: rgb(34, 34, 34); color: white; font-family: 'Helvetica Neue',Helvetica,Arial,sans-serif\">\n"
"<div style=\"height: 70px; text-align: center; padding:  10px;\">\n"
"<h2 style=\"color: #fff;\">BH PZEM - Configuration Panel v1</h2>\n"
"</div>\n"
"<div class=\"container\">\n"
"    <div class=\"row\">\n"
"        <div class=\"col-sm\">\n"
"            <div class=\"box-config\">\n"
"                <div style=\"background: #333;\">\n"
"                    <h3 style=\"color: white\">NODE CONFIG</h3>\n"
"                </div>\n"
"                <label for=\"node_id\">NODE ID</label>\n"
"                <input type=\"text\" id=\"node_id\" placeholder=\"ex: Pzem Solar\">\n"
"                <label for=\"delay_notification\">Notification interval</label>\n"
"                <select id=\"delay_notification\" >\n"
"                    <option value=\"2\">2 seconds</option>\n"
"                    <option value=\"5\">5 seconds</option>\n"
"                    <option value=\"10\">10 seconds</option>\n"
"                    <option value=\"15\">15 seconds</option>\n"
"                </select>\n"
"                <div class=\"form-check\">\n"
"                    <input type=\"checkbox\" class=\"form-check-input\" id=\"exampleCheck1\">\n"
"                    <label class=\"form-check-label\" for=\"exampleCheck1\">Detect Current Way</label>\n"
"                </div>\n"
"                <div style=\"background: inherit;\" class=\"card\">\n"
"                    <div class=\"card-header\">\n"
"                        Readings:\n"
"                    </div>\n"
"                    <ul style=\"background: inherit;\" class=\"list-group list-group-flush\">\n"
"                        <li style=\"background: inherit;\" class=\"list-group-item\"><h6>Power: <span class=\"badge badge-secondary\">0 W</span></h6></li>\n"
"                        <li style=\"background: inherit;\" class=\"list-group-item\"><h6>Voltage: <span class=\"badge badge-secondary\">0 V</span></h6></li>\n"
"                        <li style=\"background: inherit;\" class=\"list-group-item\"><h6>Current: <span class=\"badge badge-secondary\">0 A</span></h6></li>\n"
"                    </ul>\n"
"                </div>\n"
"\n"
"                <input type=\"submit\" value=\"Save All\">\n"
"            </div>\n"
"        </div>\n"
"        <div class=\"col-sm\">\n"
"            <div class=\"box-config\" >\n"
"                <div style=\"background: #333;\">\n"
"                    <h3 style=\"color: white\">EMONCMS</h3>\n"
"                </div>\n"
"                <label for=\"emoncms_api_url\">API Url</label>\n"
"                <input type=\"text\" id=\"emoncms_api_url\" placeholder=\"ex: emoncms.org\">\n"
"                <label for=\"emoncms_api_prefix\">API Prefix (leave black if your url have direct access)</label>\n"
"                <input type=\"text\" id=\"emoncms_api_prefix\" placeholder=\"ex: /emoncms\">\n"
"                <label for=\"emoncms_api_key\">API Key</label>\n"
"                <input type=\"text\" id=\"emoncms_api_key\" placeholder=\"ex: f4651ce9swq098d57375d960c612e081\">\n"
"            </div>\n"
"        </div>\n"
"        <div class=\"col-sm\">\n"
"            <div class=\"box-config\" s\">\n"
"                <div style=\"background: #333;\">\n"
"                    <h3 style=\"color: white\">MQTT</h3>\n"
"                </div>\n"
"                <label for=\"mqtt_ip\">Broker IP/DNS</label>\n"
"                <input type=\"text\" id=\"mqtt_ip\" placeholder=\"ex: 192.168.1.30\">\n"
"                <label for=\"mqtt_username\">Username</label>\n"
"                <input type=\"text\" id=\"mqtt_username\" placeholder=\"ex: iam\">\n"
"                <label for=\"mqtt_password\">Password</label>\n"
"                <input type=\"password\" id=\"mqtt_password\" placeholder=\"ex: superstrongpassword\">\n"
"            </div>\n"
"        </div>\n"
"    </div>\n"
"</div>\n"
"\n"
"</body>\n"
"</html>\n"
""};


void prepareWebserver(){
  MDNS.begin(HOSTNAME);
  httpUpdater.setup(&httpServer, update_path);

  MDNS.addService("http", "tcp", 80);
  httpServer.on("/", handleRoot);
  httpServer.onNotFound(handleNotFound);
  httpServer.begin();
  Serial.printf("BH PZEM - Configuration Panel ready! Open http://%s.localin your browser'\n", HOSTNAME);
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

