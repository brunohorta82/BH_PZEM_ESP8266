#include <Ticker.h>
Ticker pulseTicker;


void turnOn(int gpio, bool inverted) {
  pinMode(gpio,OUTPUT);
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

/*  topicStr.replace("/set","/status");
  if(payloadStr.equals(PAYLOAD_ON)){
    inverted  ? turnOnInverted(gpio) :   turnOnNormal(gpio);
    mqttClient.publish(topicStr.c_str(),0,true,PAYLOAD_ON);
    logger("[RELAY] ON");
  }else if (payloadStr.equals(PAYLOAD_OFF)){
   inverted  ? turnOffInverted(gpio) :   turnOffNormal(gpio);
   mqttClient.publish(topicStr.c_str(),0,true,PAYLOAD_OFF);
   logger("[RELAY] OFF");
  }else if (payloadStr.equals(PAYLOAD_PULSE_OFF_ON)){
    pulseOff(gpio,false,2);
  }else if (payloadStr.equals(PAYLOAD_PULSE_ON_OFF)){
    pulseOn(gpio,false,2);
  }*/

  /*
   * bool inverted = false;
   for(int i = 0; i <  totalAvailableGPIOs; i++){
      String relayName = availableGPIOS[i];
      if(relayName.equals(""))continue;
      if(split(relayName,'|',1).startsWith(relay)){
        gpio = split(relayName,'|',0).toInt();
        inverted = split(relayName,'|',2);
        break;
      }
    }
*/
   /*
    *   if(topicStr.startsWith(String(HARDWARE)+"/"+nodeId+"/relay")){

  String relay = split(topicStr,'/',2);
  int gpio = -1;
*/
    */
