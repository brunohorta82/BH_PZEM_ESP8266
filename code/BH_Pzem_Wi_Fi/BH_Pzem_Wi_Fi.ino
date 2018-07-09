#include "config.h"
#include <Timing.h> //https://github.com/scargill/Timing
//ESP
#include <ESP8266WiFi.h>
//Wi-Fi Manger library https://www.youtube.com/watch?v=wWO9n5DnuLA
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>//https://github.com/tzapu/WiFiManager
#ifdef EMONCMS_HTTPS
#include <WiFiClientSecure.h>
#endif

#include <PZEM004T.h> //https://github.com/olehs/PZEM004T

//TEMPERATURA
#include <OneWire.h>
#include <DallasTemperature.h> //https://github.com/milesburton/Arduino-Temperature-Control-Library

IPAddress pzemIP(192, 168, 1, 1);
PZEM004T pzem(RX_PIN, TX_PIN);
Timing timerRead;
//DALLAS
OneWire oneWire(DS18B20_PIN);
DallasTemperature DS18B20(&oneWire);

void setup() {
  
  Serial.begin(115200);
  timerRead.begin(0);
  WiFiManager wifiManager;
  //reset saved settings
  //wifiManager.resetSettings();
  /*define o tempo limite até o portal de configuração ficar novamente inátivo,
   útil para quando alteramos a password do AP*/
  wifiManager.setTimeout(180);
  if(!wifiManager.autoConnect(HOSTNAME)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  } 
  
  //PZEM SETUP
  pzem.setAddress(pzemIP);
  prepareWebserver();
  pinMode(DIRECTION_PIN,INPUT);
  #if MQTT
  Serial.println("*MQTT: ON");
  #endif
  #ifdef EMONCMS_API_KEY  and EMONCMS_HOST
  Serial.println("*EMONSCMS: ON");
  #endif
  #ifdef DEBUG
  Serial.println("*DEBUG: ON");
  #endif
  #if DETECT_DIRECTION
  Serial.println("*CURRENT DIRECTION DETECTION: ON");
  #endif
  #ifdef D_SSD1306
  setupDisplay();
  #endif
}

void loop() {
  
  
  #if MQTT
  if (checkMqttConnection()) {
    mqttLoop();
  }       
  #endif
  if (WiFi.status() == WL_CONNECTED) {
      webServerLoop() ;
      if (timerRead.onTimeout(DELAY_NOTIFICATION)){
        float v =  getVoltage();
        float i =  getCurrent();
        float p =  getPower()*directionSignal();
        float e =  getEnergy();
        float t =  requestTemperature();
      #ifdef D_SSD1306
      printOnDisplay(v,i,p);
      #endif  
      #if PRINT_TO_SERIAL_MONITOR  
        Serial.print("T; ");
        Serial.print(t); 
        Serial.print(v);  
        Serial.print("V; ");
        Serial.print(i);
        Serial.print("A; ");
        Serial.print(p);
        Serial.print("W; ");
        Serial.print(e);
        Serial.print("Wh; ");
        Serial.println();
      #endif
      String json = "{temperatura:"+String(t)+",voltagem:" + String(v) + ",amperagem:" + String(i) + ",potencia:" + String(p) + ",contador:" + String(e)+",config:" + String(CONFIG_VERSION) +"}";
      #if MQTT
        publishData(json);
       #endif
       #ifdef EMONCMS_API_KEY
          String url = String(EMONCMS_URL_PREFIX)+ "/input/post?node="+String(NODE_ID)+"&apikey="+String(EMONCMS_API_KEY)+"&json="+json;
          #if EMONCMS_PROTOCOL == 0 //HTTP
            WiFiClient clienthttp;
            if (!clienthttp.connect(EMONCMS_HOST,HTTP_PORT)) {
              Serial.println("connection failed");
              return;
            }
            clienthttp.print(String("GET ") + url + " HTTP/1.1\r\n" +
                       "Host: " + String(EMONCMS_HOST) + "\r\n" +
                       "Connection: close\r\n\r\n");
            unsigned long timeout = millis();
            while (clienthttp.available() == 0) {
              
              if (millis() - timeout > 5000) {
                Serial.println(">>> Client Timeout !");
                clienthttp.stop();
                return;
              }
            }
        #endif
        #if EMONCMS_PROTOCOL == 1 //HTTPS
           WiFiClientSecure clienthttps;
      
        if (!clienthttps.connect(EMONCMS_HOST,HTTPS_PORT)) {
          Serial.println("connection failed");
          return;
        }
      
        clienthttps.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + String(EMONCMS_HOST) + "\r\n" +
                   "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clienthttps.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            clienthttps.stop();
            return;
          }
        }
    #endif
    #endif
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



int directionSignal(){
  #if DETECT_DIRECTION
    return digitalRead(DIRECTION_PIN) ? -1 : 1;
  #else
    return 1;
  #endif
}

float getVoltage() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.voltage(pzemIP);
    i++;

  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}

float getCurrent() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.current(pzemIP);
    i++;

  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}

float getPower() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.power(pzemIP);
    i++;
  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}

float getEnergy() {
  int i = 0;
  float r = -1.0;
  do {
    r = pzem.energy(pzemIP);
    i++;

  } while ( i < MAX_ATTEMPTS && r < 0.0);
  return r;
}

