/*
   for(int i = 0; i <  totalAvailableGPIOs; i++){
      String relayName = availableGPIOS[i];
      if(relayName.equals(""))continue;
      String actuator = split(String(relayName),'|',1);
      if(actuator.startsWith("relay_")){
        String topic  = baseTopic+"/"+actuator;
        logger("[MQTT] "+topic);
        mqttClient.subscribe((topic+"/set").c_str(),0);
        mqttClient.publish(("homeassistant/switch/"+nodeId+"/"+actuator+"/config").c_str(),0,true,("{\"name\": \""+String(HARDWARE)+"_"+nodeId+"_"+actuator+"\", \"state_topic\": \""+topic+"/status\",\"availability_topic\": \""+availableTopic+"\", \"command_topic\": \""+topic+"/set\", \"retain\": true,\"payload_available\":\"1\",\"payload_not_available\":\"0\"}").c_str());
      }
    }
  
    mqttCreateTopics.attach(3,createSensors);
  */
  
  /*void createSensors(){
     logger("[MQTT] Creating Sensors");
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/counter/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_counter\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.contador }}\", \"unit_of_measurement\": \"ºkWh\",\"icon\":\"mdi:power-socket-eu\"}").c_str());   
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/voltage/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_voltage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.voltagem }}\", \"unit_of_measurement\": \"V\",\"icon\":\"mdi:power-socket-eu\"}").c_str());   
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/amperage/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_amperage\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.amperagem }}\", \"unit_of_measurement\": \"A\",\"icon\":\"mdi:power-socket-eu\"}").c_str());
     mqttClient.publish(("homeassistant/sensor/"+nodeId+"/power/config").c_str(),0,true,("{\"name\": \""+(String(HARDWARE)+"_"+nodeId)+"_power\", \"state_topic\": \""+(String(HARDWARE)+"/"+nodeId)+"/readings/status\", \"value_template\": \"{{ value_json.potencia }}\", \"unit_of_measurement\": \"W\",\"icon\":\"mdi:power-socket-eu\"}").c_str());  
     mqttCreateTopics.detach();
}
   */
