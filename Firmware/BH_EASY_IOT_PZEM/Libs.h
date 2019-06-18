#include <ESP8266httpUpdate.h>
#include <JustWifi.h> //https://github.com/xoseperez/justwifi
#include <ESP8266mDNS.h>
#include <DallasTemperature.h> // https://github.com/milesburton/Arduino-Temperature-Control-Library
#include <PubSubClient.h> //Install from Arduino IDE Library Manager
#include <ArduinoJson.h> //Install from Arduino IDE Library Manager
#include "FS.h"
#include <ESPAsyncTCP.h> //https://github.com/me-no-dev/ESPAsyncTCP
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer
#include <AsyncJson.h> //https://github.com/me-no-dev/ESPAsyncWebServer
#include <fauxmoESP.h> //https://bitbucket.org/xoseperez/fauxmoesp
#include <dht_nonblocking.h> // https://github.com/brunohorta82/DHT_nonblocking
#include <OneWire.h> // Install on Arduino IDE Library manager 
#include <vector>
#include <DebounceEvent.h> //https://github.com/xoseperez/debounceevent
#include <Timing.h> //https://github.com/scargill/Timing
