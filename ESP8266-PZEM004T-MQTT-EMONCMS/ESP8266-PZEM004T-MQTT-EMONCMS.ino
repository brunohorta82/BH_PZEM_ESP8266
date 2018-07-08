
#include <Timing.h> //https://github.com/scargill/Timing
//MQTT
#include <PubSubClient.h> //https://www.youtube.com/watch?v=GMMH6qT8_f4
//ESP
#include <ESP8266WiFi.h>
//Wi-Fi Manger library https://www.youtube.com/watch?v=wWO9n5DnuLA
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager

#include <WiFiClientSecure.h>

#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

#include <SoftwareSerial.h>

#include <PZEM004T.h> //https://github.com/olehs/PZEM004T

//TEMPERATURA
#include <OneWire.h>
#include <DallasTemperature.h> //https://github.com/milesburton/Arduino-Temperature-Control-Library

#define ONE_WIRE_BUS 12  // DS18B20 pin
#define DELAY_TEMPERATURE_NOTIFICATION 1000 //1second

//----------> CONFIGURAR O SERVIDOR MQTT
#define MQTT_BROKER_IP "192.168.187.203"
#define MQTT_BROKER_PORT 1883
#define MQTT_USERNAME "homeassistant"
#define MQTT_PASSWORD "moscasMoscas82"

//IP POR DEFEITO do PZEM 192.168.1.1
IPAddress pzemIP(192, 168, 1, 1);
PZEM004T pzem(4, 5);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
//Constantes
const String HOSTNAME  = "pzem-solar-bh";

//EMONCMS 
//-----------> Altera para a tua API KEY
const String API_KEY = "f4651ce96d4098d57375d960c612e081";
const String NODE_ID = "pzem-solar";
const char* host = "192.168.187.203";
const int httpsPort = 443;
float temperature = 0;
WiFiClient wclient;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

PubSubClient client(MQTT_BROKER_IP, MQTT_BROKER_PORT, wclient);
Timing notifyTempTimer;


void prepareWebserverUpdate(){
  MDNS.begin(HOSTNAME.c_str());
  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", HOSTNAME.c_str(), update_path, update_username, update_password);
}

void otaLoop(){
  httpServer.handleClient();
}
void setup() {
  
  Serial.begin(115200);
 
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  /*define o tempo limite até o portal de configuração ficar novamente inátivo,
   útil para quando alteramos a password do AP*/
  wifiManager.setTimeout(180);
  if(!wifiManager.autoConnect(HOSTNAME.c_str())) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  } 
  client.setCallback(callback);
  
  //PZEM SETUP
  pzem.setAddress(pzemIP);
  prepareWebserverUpdate();
}
//Chamada de recepção de mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  for (int i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
  Serial.println(payloadStr);
  String topicStr = String(topic);
}


//Verifica se a ligação está ativa, caso não este liga-se e subscreve aos tópicos de interesse
bool checkMqttConnection(){
  if (!client.connected()) {
    if (client.connect(HOSTNAME.c_str(),MQTT_USERNAME, MQTT_PASSWORD)) {
    }
  }
  return client.connected();
}

void loop() {
  float v = _max(pzem.voltage(pzemIP),0.0);
  float i =  _max(pzem.current(pzemIP),0.0);
  float p =  _max(pzem.power(pzemIP),0.0);
  float e =  _max(pzem.energy(pzemIP),0.0);
  
  if (WiFi.status() == WL_CONNECTED) {
    otaLoop();
    //if (checkMqttConnection()) {
     // client.loop();

      if (notifyTempTimer.onTimeout(DELAY_TEMPERATURE_NOTIFICATION)) {
        temperature = requestTemperature();
      }   
   
 
      String voltagem = String(v);
      String amperagem = String(i);
      String potencia = String(p);
      String energia = String(e);
      String temp = String(temperature);
     /* Serial.print(temp); 
      Serial.print("T; ");
      Serial.print(v);  
      Serial.print("V; ");
      Serial.print(i);
      Serial.print("A; ");
      Serial.print(p);
      Serial.print("W; ");
      Serial.print(e);
      Serial.print("Wh; ");
      Serial.println();*/
      
     
      
     // client.publish("/pzem1/energy", energia.c_str());
     // client.publish("/pzem1/power", potencia.c_str());
     // client.publish("/pzem1/amperage", amperagem.c_str());
     // client.publish("/pzem1/voltage", voltagem.c_str());
     // client.publish("/pzem1/temperature", temp.c_str());
      
      if(!API_KEY.equals("")){
         WiFiClient clienthttp;
      
        if (!clienthttp.connect(host,80)) {
          Serial.println("connection failed");
          return;
        }
      
        String url = "/emoncms/input/post?node="+NODE_ID+"&apikey="+API_KEY+"&json={temperature:"+temp+",voltagem:" + voltagem + ",amperagem:" + amperagem + ",potencia:" + potencia + ",energia:" + energia+ "}";
        clienthttp.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clienthttp.available() == 0) {
          client.loop();
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            clienthttp.stop();
            return;
          }
        }
        /**
        WiFiClientSecure clienthttps;
      
        if (!clienthttps.connect(host,httpsPort)) {
          Serial.println("connection failed");
          return;
        }
      
        String url = "/api/post?node="+NODE_ID+"&apikey="+API_KEY+"&json={temperature:"+temp+",voltagem:" + voltagem + ",amperagem:" + amperagem + ",potencia:" + potencia + ",energia:" + energia+ "}";
        clienthttps.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clienthttps.available() == 0) {
          client.loop();
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            clienthttps.stop();
            return;
          }
        }*/
     // }
    }
  }
}
float requestTemperature(){
  float temp = 0;
   do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}


