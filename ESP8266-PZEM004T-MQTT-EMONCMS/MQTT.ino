#if MQTT
#include <PubSubClient.h> //https://www.youtube.com/watch?v=GMMH6qT8_f4
WiFiClient wclient;
PubSubClient client(MQTT_BROKER_IP, MQTT_BROKER_PORT, wclient);
bool checkMqttConnection(){
  if(WiFi.status() == WL_CONNECTED){
    if (!client.connected()) {
      if (client.connect(HOSTNAME,MQTT_USERNAME, MQTT_PASSWORD)) {
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
void publishData(String json){
  if (checkMqttConnection()) {
    client.publish(String(MQTT_TOPIC).c_str(),json.c_str());
  }
}

#endif
