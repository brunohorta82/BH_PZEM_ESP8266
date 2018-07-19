AsyncMqttClient mqttClient; 
Ticker mqttReconnectTimer;
Ticker mqttCreateTopics;
String baseTopic = String(HARDWARE)+"/"+nodeId;
String availableTopic = String(HARDWARE)+"_"+nodeId+"/status";
void onMqttConnect(bool sessionPresent) {
     Serial.println("[MQTT] Connected to MQTT.");
     mqttClient.publish(availableTopic.c_str(),0,true,"1");
     for(int i = 0; i <  totalAvailableGPIOs; i++){
      String relayName = availableGPIOS[i];
      if(relayName.equals(""))continue;
      String actuator = split(String(relayName),'|',1);
      if(actuator.startsWith("relay_")){
        String topic  = baseTopic+"/"+actuator;
        Serial.println("[MQTT] "+topic);
        mqttClient.subscribe((topic+"/set").c_str(),0);
        mqttClient.publish(("homeassistant/switch/"+nodeId+"/"+actuator+"/config").c_str(),0,true,("{\"name\": \""+String(HARDWARE)+"_"+nodeId+"_"+actuator+"\", \"state_topic\": \""+topic+"/status\",\"availability_topic\": \""+availableTopic+"\", \"command_topic\": \""+topic+"/set\", \"retain\": true,\"payload_available\":\"1\",\"payload_not_available\":\"0\"}").c_str());
      }
    }
  
    mqttCreateTopics.attach(3,createSensors);
}
void createSensors(){
     Serial.println("[MQTT] Create Sensors");
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/counter/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_counter\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.contador }}\", \"unit_of_measurement\": \"ºkWh\",\"icon\":\"mdi:power-socket-eu\"}").c_str());   
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/voltage/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_voltage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.voltagem }}\", \"unit_of_measurement\": \"V\",\"icon\":\"mdi:power-socket-eu\"}").c_str());   
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/amperage/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_amperage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.amperagem }}\", \"unit_of_measurement\": \"A\",\"icon\":\"mdi:power-socket-eu\"}").c_str());
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/power/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_power\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.potencia }}\", \"unit_of_measurement\": \"W\",\"icon\":\"mdi:power-socket-eu\"}").c_str());  
     mqttCreateTopics.detach();
}
void onMqttPublish(uint16_t packetId) {
}

void connectToMqtt() {
  Serial.println("[MQTT] Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("[MQTT] Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}


void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //RELAYS
  String topicStr = String(topic);
  Serial.println("[MQTT] "+topicStr);
  if(topicStr.startsWith(String(HARDWARE)+"/"+nodeId+"/relay")){
     String payloadStr = "";
  for (int i=0; i<len; i++) {
    payloadStr += payload[i];
  }
  String relay = split(topicStr,'/',2);
  int gpio = -1;
  bool inverted = false;
   for(int i = 0; i <  totalAvailableGPIOs; i++){
      String relayName = availableGPIOS[i];
      if(relayName.equals(""))continue;
      if(split(relayName,'|',1).startsWith(relay)){
        gpio = split(relayName,'|',0).toInt();
        inverted = split(relayName,'|',2);
        break;
      }
    }
  topicStr.replace("/set","/status");
  if(payloadStr.equals(PAYLOAD_ON)){
    inverted  ? turnOnInverted(gpio) :   turnOnNormal(gpio);
    mqttClient.publish(topicStr.c_str(),0,true,PAYLOAD_ON);
    Serial.println("[RELAY] ON");
  }else if (payloadStr.equals(PAYLOAD_OFF)){
   inverted  ? turnOffInverted(gpio) :   turnOffNormal(gpio);
   mqttClient.publish(topicStr.c_str(),0,true,PAYLOAD_OFF);
   Serial.println("[RELAY] OFF");
  }else if (payloadStr.equals(PAYLOAD_PULSE_OFF_ON)){
    pulseOff(gpio,false,2);
  }else if (payloadStr.equals(PAYLOAD_PULSE_ON_OFF)){
    pulseOn(gpio,false,2);
  }else{
  Serial.println("[MQTT] Invalid message: "+ payloadStr);
  }
  }
}
void setupMQTT() {
  if(WiFi.status() != WL_CONNECTED || mqttIpDns.equals(""))return;
  mqttClient.disconnect();
  mqttClient.onConnect(onMqttConnect);
  mqttClient.setClientId((String(HARDWARE)+"_"+nodeId).c_str());
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setCredentials(mqttUsername.c_str(),mqttPassword.c_str());
  mqttClient.setCleanSession(false);
  mqttClient.setWill(availableTopic.c_str(),0,true,"0");
  mqttClient.setServer( mqttIpDns.c_str(), MQTT_BROKER_PORT);
 connectToMqtt();
}

  void publishOnMqtt(String json){
     mqttClient.publish((String(HARDWARE)+"/"+String(nodeId)+"/readings/status").c_str(), 0,false,json.c_str());
 }
