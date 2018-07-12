#include <Ticker.h>
Ticker pulseTicker;


void turnOn(int gpio, bool inverted) {
  pinMode(gpio,OUTPUT);
  Serial.println(gpio);
 if(inverted){
      digitalWrite( gpio,LOW);
    }else{
      digitalWrite( gpio,HIGH);
    }
}

void turnOff(int gpio, bool inverted) {
    pinMode(gpio,OUTPUT);
  if(inverted){
      digitalWrite( gpio,HIGH);
    }else{
      digitalWrite( gpio,LOW);
    }
}

void turnOnInverted(int gpio) {
  turnOn(gpio,true);
  
}

void turnOffInverted(int gpio) {
  turnOff(gpio,true);
}

void turnOnNormal(int gpio) {
  turnOn(gpio,false);
}

void turnOffNormal(int gpio) {
  turnOff(gpio,false);
}

// FIRST ON AFTER DELAY OFF
void pulseOn(int gpio, bool inverted, int delay) {
  if(inverted){
     turnOnInverted(gpio);
     pulseTicker.once(delay, turnOffInverted,gpio);
    }else{
       turnOnNormal(gpio);
       pulseTicker.once(delay, turnOffNormal,gpio);
    } 
}

// FIRST OFF AFTER DELAY ON
void pulseOff(int gpio, bool inverted, int delay) {
  if(inverted){
     turnOffInverted(gpio);
     pulseTicker.once(delay, turnOnInverted,gpio);
    }else{
       turnOffNormal(gpio);
       pulseTicker.once(delay, turnOnNormal,gpio);
    } 
}
