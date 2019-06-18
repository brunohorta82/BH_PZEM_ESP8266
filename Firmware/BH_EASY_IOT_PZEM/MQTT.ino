WiFiClient espClient;
PubSubClient mqttClient(espClient);
String getBaseTopic()
{
  String username = getConfigJson().get<String>("mqttUsername");
  if (username == "")
  {
    username = String(HARDWARE);
  }
  return username + "/" + String(ESP.getChipId());
}
String getAvailableTopic()
{
  return getBaseTopic() + "/available";
}

boolean reconnect() {
  
  if (WiFi.status() != WL_CONNECTED || getConfigJson().get<String>("mqttIpDns").equals(""))
    return false;
    logger("[MQTT] TRY CONNECTION");
    char *username = strdup(getConfigJson().get<String>("mqttUsername").c_str());
    char *password = strdup(getConfigJson().get<String>("mqttPassword").c_str());
  if (mqttClient.connect(String(ESP.getChipId()).c_str(),username,password,getAvailableTopic().c_str(), 0, true, "offline", false)) {
      logger("[MQTT] CONNECTED");
      publishOnMqtt(getAvailableTopic().c_str() ,"online",true);
      reloadMqttSubscriptionsAndDiscovery();
  }
  
  return mqttClient.connected();
}
void rebuildSwitchMqttTopics(JsonObject &switchJson)
{
  String ipMqtt = getConfigJson().get<String>("mqttIpDns");
  if (ipMqtt == "")
    return;
    String _id = switchJson.get<String>("id");
    String type = switchJson.get<String>("type");
    String _class = switchJson.get<String>("class");
    switchJson.set("mqttCommandTopic", getBaseTopic() + "/" + _class + "/" + _id + "/set");
    switchJson.set("mqttStateTopic", getBaseTopic() + "/" + _class + "/" + _id + "/state");
    if (type.equals("cover"))
    {
      switchJson.set("mqttPositionStateTopic", getBaseTopic() + "/" + _class + "/" + _id + "/position");
      switchJson.set("mqttPositionCommandTopic", getBaseTopic() + "/" + _class + "/" + _id + "/setposition");
      switchJson.set("mqttPositionStateTopic", getBaseTopic() + "/" + _class + "/" + _id + "/position");
      switchJson.set("mqttTiltStateTopic", getBaseTopic() + "/" + _class + "/" + _id + "/tiltstate");
      switchJson.set("mqttTiltCommandTopic", getBaseTopic() + "/" + _class + "/" + _id + "/tilt");
    }
}
void rebuildSensorMqttTopics(JsonObject &sensorJson) {
String ipMqtt = getConfigJson().get<String>("mqttIpDns");
  if (ipMqtt == "")
    return;  
      String _id = sensorJson.get<String>("id");
      String _class = sensorJson.get<String>("class");
      sensorJson.set("mqttStateTopic", getBaseTopic() + "/" + _class + "/" + _id + "/state");
}

void setupMQTT()
{
  logger("[MQTT] SETUP MQTT");
  if(mqttClient.connected()){
   mqttClient.disconnect(); 
   }
   char *ipDnsMqtt = strdup(getConfigJson().get<String>("mqttIpDns").c_str());
   int port = getConfigJson().get<unsigned int>("mqttPort");
   
   mqttClient.setServer(ipDnsMqtt,port);
   mqttClient.setCallback(callbackMqtt);
}
void callbackMqtt(char* topic, byte* payload, unsigned int length) {
String topicStr = String(topic);
  logger("[MQTT] TOPIC: " + topicStr);
  String payloadStr = "";
  for (int i = 0; i < length; i++)
  {
    payloadStr += payload[i];
  }
  logger("[MQTT] PAYLOAD: " + payloadStr);
  processMqttAction(topicStr, payloadStr);
}


void loopMqtt(){
   static unsigned long lastReconnectAttempt = millis( );
  if (!mqttClient.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      if (reconnect()) {
        lastReconnectAttempt = 0;
        
      }
    }
  } else {
    mqttClient.loop();
  }
  
  }



void publishOnMqtt(String topic, String payload, bool retain)
{
  if (mqttClient.connected())
  {
    mqttClient.publish(topic.c_str(), payload.c_str(), retain);
  }
}

void subscribeOnMqtt(String topic)
{
  mqttClient.subscribe(topic.c_str(), 0);
}
void processMqttAction(String topic, String payload)
{
  mqttSwitchControl(topic, payload);
}
