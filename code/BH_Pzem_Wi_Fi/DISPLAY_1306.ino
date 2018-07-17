#include <Wire.h>  
#include "SSD1306.h" //https://github.com/ThingPulse/esp8266-oled-ssd1306

SSD1306 display(0x3c,displaySDA,displaySCL);

void setupDisplay(){
  displaySDA = -1;
   displaySCL = -1;
  for(int i = 0; i <  totalAvailableGPIOs; i++){
      String gpioConfig = availableGPIOS[i];
      if(gpioConfig.equals(""))continue;
      String deviceTarget = split(String(gpioConfig),'|',1);
      if(deviceTarget.equals("DISPLAY")){
        String gpioFunction = split(String(gpioConfig),'|',2);
        if(gpioFunction.equals("SDA")){
          displaySDA = split(String(gpioConfig),'|',0).toInt();
        }  else if(gpioFunction.equals("SCL")){
          displaySCL = split(String(gpioConfig),'|',0).toInt();
        }
      }
      }

  if(displaySDA == -1 || displaySCL == -1)return;
    Serial.println("[DISPLAY] INIT..");
    Serial.print("[DISPLAY] SDA: ");
    Serial.println(displaySDA);
    Serial.print("[DISPLAY] SCL: ");
    Serial.println(displaySCL);
    display = SSD1306Wire(0x3c,displaySDA,displaySCL);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(5,0, "BH PZEM");
    display.display();
  
}

void printOnDisplay(float _voltage, float _amperage, float _power, float _energy, String _temperatures){
  if(displaySDA == -1 || displaySCL == -1)return;
  
  display.clear(); 
  display.setFont(ArialMT_Plain_16);
  display.drawString(5,0, String(_power)+"W");
  display.setFont(ArialMT_Plain_10);
  display.drawString(5,16, String(_energy)+" kWh");
  display.drawString(5,26, String(_voltage)+" V");
  display.drawString(5,36, String(_amperage)+" A");
  display.drawString(5,46, String(_temperatures));
  display.display();
}

