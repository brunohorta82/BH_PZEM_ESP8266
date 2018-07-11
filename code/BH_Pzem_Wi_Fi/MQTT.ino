
#include <PubSubClient.h> //https://www.youtube.com/watch?v=GMMH6qT8_f4
WiFiClient wclient;
PubSubClient client(mqttIpDns.c_str(), MQTT_BROKER_PORT, wclient);
bool checkMqttConnection(){
  if(mqttIpDns.equals(""))
    return false;
  if(WiFi.status() == WL_CONNECTED){
    if (!client.connected()) {
      if (client.connect(HOSTNAME, mqttUsername.c_str(),mqttPassword.c_str())) {
        #if PRINT_TO_SERIAL_MONITOR
        Serial.println("CONNECTED ON MQTT");
        #endif
      }
    }
    return client.connected();
    }
  return false;
}

void mqttLoop(){
  client.loop();
}
void publishOnMqtt(String json){
  if (checkMqttConnection()) {
    client.publish(String(MQTT_TOPIC).c_str(),json.c_str());
  }
}


