#ifdef D_SSD1306
#include <Wire.h>  
#include "SSD1306.h"
SSD1306  display(0x3c, SDA_PIN, SCL_PIN);
void setupDisplay(){
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
}

void printOnDisplay(float voltagem, float amperage, float power){
  Serial.println("Print");
 // display.clear(); // clearing the display
  display.drawString(5,0, "BH PZEM 1.0");
  //display.drawString(3,1, String(voltagem)+"V");
  //display.drawString(3,2, String(amperage)+"A");
  //display.drawString(5,0, String(power)+"W");
  display.display();
}
#endif
