#include <Ticker.h>
Ticker pulseTicker;
void turnOn(int gpio, bool inverted) {
 if(inverted){
      turnOnInverted( gpio);
    }else{
      turnOnNormal( gpio);
    }
}

void turnOff(int gpio, bool inverted) {
  if(inverted){
      turnOffInverted( gpio);
    }else{
      turnOffNormal( gpio);
    }
}

void turnOnInverted(int gpio) {
  digitalWrite(gpio, 0);
}

void turnOffInverted(int gpio) {
  digitalWrite(gpio,  1);
}

void turnOnNormal(int gpio) {
  digitalWrite(gpio, 1);
}

void turnOffNormal(int gpio) {
  digitalWrite(gpio,  0);
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
