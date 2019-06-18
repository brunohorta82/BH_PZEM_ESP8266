/*

BH FIRMWARE

Copyright (C) 2017-2018 by Bruno Horta <brunohorta82 at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "Libs.h"
#include "Config.h"

void checkInternalRoutines()
{
  if (REBOOT)
  {
    static unsigned long timestamp = millis( ); 
    if( millis( ) -timestamp > 4000ul )
  {
     logger("Rebooting...");
    timestamp = millis( );
    REBOOT  = false;
    ESP.restart();
  }
    return;
  }
  
  if (LOAD_DEFAULTS)
  {
    LOAD_DEFAULTS = false;
    SPIFFS.format();
    requestReboot();
  }
  if (WIFI_SCAN)
  {
    WIFI_SCAN = false;
    scanNewWifiNetworks();
    
  }
  if(STORE_CONFIG){
    STORE_CONFIG = false;
    persistConfigFile();
    }
  if (AUTO_UPDATE)
  {
    AUTO_UPDATE = false;
    actualUpdate();
  }
  
  
}
void setup() {
  Serial.begin(115200);
  loadStoredConfiguration();
  setupWiFi(); 
  setupWebserver(); 
  setupBHPzem();
  setupDisplay();
  reloadAlexaDiscoveryServices();
  
}

void loop() {
  checkInternalRoutines();
  MDNS.update();
  loopBHPzem();
  loopWiFi();
   loopMqtt();
  
}

void actualUpdate(){
  WiFiClient client;
     ESPhttpUpdate.update(client, getUpdateUrl());
 
}
