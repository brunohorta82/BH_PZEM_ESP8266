#include <WiFiClientSecure.h>

void publishOnEmoncms(String json){
  if(WiFi.status() != WL_CONNECTED || emoncmsUrl.equals("") || emoncmsApiKey.equals(""))return;
  
     String url = emoncmsPrefix+ "/input/post?node="+nodeId+"&apikey="+emoncmsApiKey+"&json="+json;
          if(emoncmshttp){
            WiFiClient clienthttp;
            if (!clienthttp.connect(emoncmsUrl,HTTP_PORT)) {
              Serial.println("connection failed");
              
              return;
            }
            clienthttp.print(String("GET ") + url + " HTTP/1.1\r\n" +
                       "Host: " + emoncmsUrl + "\r\n" +
                       "Connection: close\r\n\r\n");
            unsigned long timeout = millis();
            while (clienthttp.available() == 0) {
              if (millis() - timeout > 5000) {
                Serial.println(">>> Client Timeout !");
                clienthttp.stop();
                return;
              }
            }
          }else{
        
           WiFiClientSecure clienthttps;
      
        if (!clienthttps.connect(emoncmsUrl,HTTPS_PORT)) {
          Serial.println("connection failed");
         
          return;
        }
      
        clienthttps.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + String(emoncmsUrl) + "\r\n" +
                   "Connection: close\r\n\r\n");
        unsigned long timeout = millis();
        while (clienthttps.available() == 0) {
          if (millis() - timeout > 5000) {
            Serial.println(">>> Client Timeout !");
            clienthttps.stop();
            return;
          }
        }
       }
}
