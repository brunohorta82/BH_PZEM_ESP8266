#include <JustWifi.h>
#include "config.h"
#include "static_site.h"
#include <Timing.h> //https://github.com/scargill/Timing
#include <AsyncMqttClient.h> //https://github.com/marvinroger/async-mqtt-client
#include <PZEM004T.h> //https://github.com/olehs/PZEM004T
//TEMPERATURA
#include <OneWire.h>
#include <DallasTemperature.h> //https://github.com/milesburton/Arduino-Temperature-Control-Library
DeviceAddress sensores[8];
IPAddress pzemIP(192, 168, 1, 1);
PZEM004T pzem(RX_PIN, TX_PIN);
Timing timerRead;
//DALLAS
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress devAddr[15];  // array of (up to) 15 temperature sensors
String devAddrNames[15];  // array of (up to) 15 temperature sensors
int sensorsCount = 0;

void setup() {
  Serial.begin(115200);
  loadStoredConfiguration();
  jw.setHostname(hostname.c_str());
  jw.subscribe(infoCallback);
  jw.enableAP(false);
  jw.enableAPFallback(true);
  jw.enableSTA(true);

  jw.addNetwork(wifiSSID.c_str(), wifiSecret.c_str());

  timerRead.begin(0);
  prepareWebserver();
  //PZEM SETUP
  pzem.setAddress(pzemIP);
  pinMode(DIRECTION_PIN,INPUT);
  #ifdef D_SSD1306
  setupDisplay();
  #endif
  sensors.begin();
  sensorsCount = sensors.getDeviceCount();
  oneWire.reset_search();
  for (int i=0; i<sensorsCount; i++){
   if (!oneWire.search(devAddr[i])) 
   Serial.print("Unable to find temperature sensors address "); Serial.println(i);
  }
  for (int a=0; a<sensorsCount; a++){ 
  String addr  = "";
    for (uint8_t i = 0; i < 8; i++){
      if (devAddr[0][i] < 16) addr+="0";
      addr+=String(devAddr[a][i], HEX);
   }
  devAddrNames[a] = addr;
  }
}

void loop() {
  if(shouldReboot){
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
    return;
  }
    
      jw.loop();
      checkConfigChanges();
      if(restartMqtt){
        restartMqtt = false;
        setupMQTT() ;
       }
      float t = 0;

      if (timerRead.onTimeout(notificationInterval)){
        float v = getVoltage();
        float i = getCurrent();
        float p =  getPower()*directionSignal();
        float e = getEnergy()/1000;
        sensors.requestTemperatures();
        String temperatures= "";
        for(int a = 0 ;a < sensorsCount; a++){
          temperatures += "\"temp_"+devAddrNames[a]+"\":"+String(requestTemperature(devAddr[a]))+",";
        }
        
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
        Serial.print("kWh; ");
        Serial.println();
      #endif
      String json = "{"+ temperatures+"\"voltagem\":" + String(v) + ",\"amperagem\":" + String(i) + ",\"potencia\":" + String(p) + ",\"contador\":" + String(e)+",\"config\":" + String(FIRMWARE_VERSION) +"}";
      publishOnPanel(json);
      publishOnMqtt(json);
      publishOnEmoncms(json);
    }
  
}

float requestTemperature(DeviceAddress deviceAddress){
  float temp = 0;
   do {
    temp = sensors.getTempC( deviceAddress);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}



int directionSignal(){
  if(directionCurrentDetection)
    return digitalRead(DIRECTION_PIN) ? -1 : 1;
  return 1;
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


