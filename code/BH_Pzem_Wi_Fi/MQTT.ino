
#include <Ticker.h>


AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

Ticker wifiReconnectTimer;

void onMqttConnect(bool sessionPresent) {
  Serial.println("[MQTT] Connected to MQTT.");
  for(int i = 0; i < relayCount; i++){
    mqttClient.subscribe(("bhpzem/"+nodeId+"/relay_"+String(i)).c_str(),0);
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
Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
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
     mqttClient.publish(String(MQTT_TOPIC).c_str(), 0,false,json.c_str());
 }
