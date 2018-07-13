
#include <Ticker.h>
#include <ArduinoJson.h>

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

Ticker wifiReconnectTimer;

void onMqttConnect(bool sessionPresent) {
  Serial.println("[MQTT] Connected to MQTT.");
 
    for(int i = 0; i <  totalAvailableGPIOs; i++){
      String relayName = availableGPIOS[i];
      if(relayName.equals(""))continue;
      String actuator = getValue(String(relayName),'|',1);
      if(actuator.startsWith("relay_")){
        String topic  = String(HARDWARE)+"/"+nodeId+"/"+actuator;
        Serial.println("[MQTT] "+topic);
        mqttClient.subscribe((topic+"/set").c_str(),0);
        mqttClient.publish(("homeassistant/switch/"+nodeId+"/"+actuator+"/config").c_str(),0,true,("{\"name\": \""+String(HARDWARE)+"_"+nodeId+"_"+actuator+"\", \"state_topic\": \""+topic+"/status\", \"command_topic\": \""+topic+"/set\", \"retain\": true}").c_str());
        
      }
    }
}
void connectToMqtt() {
  Serial.println("[MQTT] Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}


void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //RELAYS
  String topicStr = String(topic);
  Serial.println(topicStr);
  if(topicStr.startsWith(String(HARDWARE)+"/"+nodeId+"/relay")){
     String payloadStr = "";
  for (int i=0; i<len; i++) {
    payloadStr += payload[i];
  }
  String relay = getValue(topicStr,'/',2);
  int gpio = -1;
  bool inverted = false;
   for(int i = 0; i <  totalAvailableGPIOs; i++){
      String relayName = availableGPIOS[i];
      if(relayName.equals(""))continue;
      if(getValue(relayName,'|',1).startsWith(relay)){
        gpio = getValue(relayName,'|',0).toInt();
        inverted = getValue(relayName,'|',2);
        break;
      }
    }
  topicStr.replace("/set","/status");
  if(payloadStr.equals("ON")){
    inverted  ? turnOnInverted(gpio) :   turnOnNormal(gpio);
    mqttClient.publish(topicStr.c_str(),0,true,"ON");
  }else if (payloadStr.equals("OFF")){
   inverted  ? turnOffInverted(gpio) :   turnOffNormal(gpio);
   mqttClient.publish(topicStr.c_str(),0,true,"OFF");
  }else if (payloadStr.equals("OFF_PULSE")){
    pulseOff(gpio,false,2);
  }else if (payloadStr.equals("ON_PULSE")){
    pulseOn(gpio,false,2);
  }
  Serial.println( payloadStr);
  }
}
void setupMQTT() {
  if(WiFi.status() != WL_CONNECTED || mqttIpDns.equals(""))return;
  mqttClient.disconnect();
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setCredentials(mqttUsername.c_str(),mqttPassword.c_str());
  mqttClient.setServer( mqttIpDns.c_str(), MQTT_BROKER_PORT);
 connectToMqtt();
}

  void publishOnMqtt(String json){
     mqttClient.publish((String(HARDWARE)+"/"+String(NODE_ID)+"/readings/status").c_str(), 0,false,json.c_str());
 }
