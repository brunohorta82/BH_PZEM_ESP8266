
#include <Wire.h>  
#include "SSD1306.h" //https://github.com/ThingPulse/esp8266-oled-ssd1306


SSD1306 display(0x3c, DISPLAY_SDA,DISPLAY_SCL);

void setupDisplay(){
    pinMode(0,INPUT);
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(5,0, "BH PZEM");
    display.display();
    
  
}
void printOnDisplay(float _voltage, float _amperage, float _power, float _energy, String _temperatures){
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

void loopSwitchDisplay(){
  if(digitalRead(0)){
    display.displayOff();
    Serial.println("OFF");
    }else{
      display.displayOn();
      Serial.println("ON");
      }
  }



