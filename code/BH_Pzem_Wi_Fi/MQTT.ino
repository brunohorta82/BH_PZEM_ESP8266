AsyncMqttClient mqttClient; 
Ticker mqttReconnectTimer;
Ticker mqttCreateTopics;
const char* topics[] = {("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_counter\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.contador }}\", \"unit_of_measurement\": \"ºkWh\",\"icon\":\"mdi:power-socket-eu\"}").c_str(),
("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_voltage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.voltagem }}\", \"unit_of_measurement\": \"V\",\"icon\":\"mdi:power-socket-eu\"}").c_str(),
("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_amperage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.amperagem }}\", \"unit_of_measurement\": \"A\",\"icon\":\"mdi:power-socket-eu\"}").c_str(),
("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_power\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.potencia }}\", \"unit_of_measurement\": \"W\",\"icon\":\"mdi:power-socket-eu\"}").c_str()};

void onMqttConnect(bool sessionPresent) {
     Serial.println("[MQTT] Connected to MQTT.");
     for(int i = 0; i <  totalAvailableGPIOs; i++){
      String relayName = availableGPIOS[i];
      if(relayName.equals(""))continue;
      String actuator = split(String(relayName),'|',1);
      if(actuator.startsWith("relay_")){
        String topic  = String(HARDWARE)+"/"+nodeId+"/"+actuator;
        Serial.println("[MQTT] "+topic);
        mqttClient.subscribe((topic+"/set").c_str(),0);
        mqttClient.publish(("homeassistant/switch/"+nodeId+"/"+actuator+"/config").c_str(),1,true,("{\"name\": \""+String(HARDWARE)+"_"+nodeId+"_"+actuator+"\", \"state_topic\": \""+topic+"/status\", \"command_topic\": \""+topic+"/set\", \"retain\": true}").c_str());
      }
    }
  
    mqttCreateTopics.attach(3,createSensors);
}
void createSensors(){
     Serial.println("[MQTT] Create Sensors");
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/counter/config").c_str(),1,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_counter\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.contador }}\", \"unit_of_measurement\": \"ºkWh\",\"icon\":\"mdi:power-socket-eu\"}").c_str());   
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/voltage/config").c_str(),1,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_voltage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.voltagem }}\", \"unit_of_measurement\": \"V\",\"icon\":\"mdi:power-socket-eu\"}").c_str());   
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/amperage/config").c_str(),1,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_amperage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.amperagem }}\", \"unit_of_measurement\": \"A\",\"icon\":\"mdi:power-socket-eu\"}").c_str());
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/power/config").c_str(),1,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_power\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.potencia }}\", \"unit_of_measurement\": \"W\",\"icon\":\"mdi:power-socket-eu\"}").c_str());  
     mqttCreateTopics.detach();
}
void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
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
  Serial.println(topicStr);
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
    mqttClient.publish(topicStr.c_str(),0,true,"ON");
  }else if (payloadStr.equals(PAYLOAD_OFF)){
   inverted  ? turnOffInverted(gpio) :   turnOffNormal(gpio);
   mqttClient.publish(topicStr.c_str(),0,true,"OFF");
  }else if (payloadStr.equals(PAYLOAD_PULSE_OFF_ON)){
    pulseOff(gpio,false,2);
  }else if (payloadStr.equals(PAYLOAD_PULSE_ON_OFF)){
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
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setCredentials(mqttUsername.c_str(),mqttPassword.c_str());
  mqttClient.setServer( mqttIpDns.c_str(), MQTT_BROKER_PORT);
 connectToMqtt();
}

  void publishOnMqtt(String json){
     mqttClient.publish((String(HARDWARE)+"/"+String(nodeId)+"/readings/status").c_str(), 0,false,json.c_str());
 }
