#include <Wire.h>  
#include "SSD1306.h" //https://github.com/ThingPulse/esp8266-oled-ssd1306

SSD1306 display(0x3c, displaySDA, displaySCL);

void setupDisplay(){
  displaySDA = -1;
  displaySCL = -1;
  for(int i = 0; i <  totalAvailableGPIOs; i++){
      String gpioConfig = availableGPIOS[i];
      if(gpioConfig.equals(""))continue;
      String deviceTarget = split(String(deviceTarget),'|',1);
      if(deviceTarget.equals("DISPLAY")){
        String gpioFunction = split(String(deviceTarget),'|',2);
        if(gpioFunction.equals("SDA")){
          displaySDA = gpioFunction.toInt();
        }  else if(gpioFunction.equals("SCL")){
          displaySCL = gpioFunction.toInt();
        }
      }
      }
      
  if(displaySDA == -1 || displaySCL == -1)return;
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
  
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

