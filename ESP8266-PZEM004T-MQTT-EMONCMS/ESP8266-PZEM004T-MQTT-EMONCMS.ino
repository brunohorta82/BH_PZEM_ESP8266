
#include <Timing.h> //https://github.com/scargill/Timing
//MQTT
#include <PubSubClient.h>
//ESP
#include <ESP8266WiFi.h>
//Wi-Fi Manger library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager
//OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>  
#include <WiFiClientSecure.h>

#include <SoftwareSerial.h>
//https://github.com/olehs/PZEM004T
#include <PZEM004T.h>
//TEMPERATURA
#include <OneWire.h>
#include <DallasTemperature.h> //https://github.com/milesburton/Arduino-Temperature-Control-Library

#define ONE_WIRE_BUS 12  // DS18B20 pin
#define DELAY_TEMPERATURE_NOTIFICATION 1000 //1second

//----------> CONFIGURAR O SERVIDOR MQTT
#define MQTT_BROKER_IP "0.0.0.0"
#define MQTT_BROKER_PORT 1883
#define MQTT_AUTH false
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""

//IP POR DEFEITO do PZEM 192.168.1.1
IPAddress pzemIP(192, 168, 1, 1);
PZEM004T pzem(4, 5);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
//CONTROL FLAGS
bool OTA = false;
bool OTABegin = false;
//Constantes
const String HOSTNAME  = "pzem-bh";
const char * OTA_PASSWORD  = "otapower";

//-----------> Altera para o teu broker mqtt
const String MQTT_LOG = "system/log";
const String MQTT_SYSTEM_CONTROL_TOPIC = "system/set/"+HOSTNAME;

//EMONCMS 
//-----------> Altera para a tua API KEY
const String API_KEY = "API_KEY";
const String NODE_ID = "pzem";
const char* host = "emoncms.org";
const int httpsPort = 443;
//--------- Altera o Finger print se necessário https://www.youtube.com/watch?v=RgCi0luav1I
const char* fingerprint = "1D 08 43 BC B4 9C FB B1 61 37 F7 05 D6 6B B7 38 28 93 26 E6";

WiFiClient wclient;

PubSubClient client(MQTT_BROKER_IP, MQTT_BROKER_PORT, wclient);
Timing notifyTempTimer;
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
}
//Chamada de recepção de mensagens MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String payloadStr = "";
  for (int i=0; i<length; i++) {
    payloadStr += (char)payload[i];
  }
  Serial.println(payloadStr);
  String topicStr = String(topic);
  if(topicStr.equals(MQTT_SYSTEM_CONTROL_TOPIC)){
    if(payloadStr.equals("OTA_ON_"+String(HOSTNAME))){
      OTA = true;
      OTABegin = true;
    }else if (payloadStr.equals("OTA_OFF_"+String(HOSTNAME))){
      OTA = true;
      OTABegin = true;
    }else if (payloadStr.equals("REBOOT_"+String(HOSTNAME))){
      ESP.restart();
    }
  } 
}


//Verifica se a ligação está ativa, caso não este liga-se e subscreve aos tópicos de interesse
bool checkMqttConnection(){
  if (!client.connected()) {
    if (MQTT_AUTH ? client.connect(HOSTNAME.c_str(),MQTT_USERNAME, MQTT_PASSWORD) : client.connect(HOSTNAME.c_str())) {
      //SUBSCRIÇÃO DE TOPICOS
      Serial.println("CONNECTED ON MQTT");
      client.subscribe(MQTT_SYSTEM_CONTROL_TOPIC.c_str());
      //Envia uma mensagem por MQTT para o tópico de log a informar que está ligado
      client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" CONNECTED").c_str());
    }
  }
  return client.connected();
}

void loop() {
  float v = pzem.voltage(pzemIP);
  float i = pzem.current(pzemIP);
  float p = pzem.power(pzemIP); 
  float e = pzem.energy(pzemIP);
  
  if (WiFi.status() == WL_CONNECTED) {
    if (checkMqttConnection()) {
      client.loop();

      if (notifyTempTimer.onTimeout(DELAY_TEMPERATURE_NOTIFICATION)) {
        notifyTemperature();
      }   
      if(OTA){
        if(OTABegin){
          setupOTA();
          OTABegin= false;
        }
        ArduinoOTA.handle();
      }

      if (v < 0 || i < 0 || p < 0 || e < 0) return;

      
      String voltagem = String(v);
      String amperagem = String(i);
      String potencia = String(p);
      String energia = String(e);
      Serial.print(v); 
      Serial.print("V; ");
      Serial.print(i);
      Serial.print("A; ");
      Serial.print(p);
      Serial.print("W; ");
      Serial.print(e);
      Serial.print("Wh; ");
      Serial.println();
      
     
      
      client.publish("/pzem/energy", energia.c_str());
      client.publish("/pzem/power", potencia.c_str());
      client.publish("/pzem/amperage", amperagem.c_str());
      client.publish("/pzem/voltage", voltagem.c_str());

    // Use WiFiClientSecure class to create TLS connection
      WiFiClientSecure clienthttps;
      
      if (!clienthttps.connect(host,httpsPort)) {
        Serial.println("connection failed");
        return;
      }
      client.loop();
      String url = "/api/post?node="+NODE_ID+"&apikey="+API_KEY+"&json={voltagem:" + String(v) + ",amperagem:" + String(i) + ",potencia:" + String(p) + ",energia:" + String(e)+ "}";
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
      }

    }
  }
}
void notifyTemperature(){
  float temp = 0;
   do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    Serial.print("Temperature: ");
    Serial.println(temp);
  } while (temp == 85.0 || temp == (-127.0));
}
//Setup do OTA para permitir updates de Firmware via Wi-Fi
void setupOTA(){
  if (WiFi.status() == WL_CONNECTED && checkMqttConnection()) {
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS SETUP").c_str());
    ArduinoOTA.setHostname(HOSTNAME.c_str());
    ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
    ArduinoOTA.begin();
    client.publish(MQTT_LOG.c_str(),(String(HOSTNAME)+" OTA IS READY").c_str());
  }  
}
