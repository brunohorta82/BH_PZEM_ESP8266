void scanNewWifiNetworks(){
  jw.enableScan(true);
  }
void setupWiFi(){
  jw.setHostname(hostname.c_str());
  jw.subscribe(infoCallback);
  jw.enableAP(false);
  jw.enableAPFallback(true);
  jw.enableSTA(true);
  jw.enableScan(true);
  jw.cleanNetworks();
  jw.addNetwork(wifiSSID.c_str(), wifiSecret.c_str());
}
void loopWiFi(){
  jw.loop();
}
