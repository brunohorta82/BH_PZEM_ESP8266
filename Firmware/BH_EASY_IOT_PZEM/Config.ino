AsyncEventSource events("/events");

JsonObject &configJson = getJsonObject();
typedef struct {
    int gpio;
} gpios_t;
std::vector <gpios_t> inUseGpios;

void logger(String payload) {
    if (payload.equals(""))return;

    Serial.println(payload);
}


void resetToFactoryConfig() {
    SPIFFS.format();
    shouldReboot = true;
}

JsonObject &getConfigJson() {
    return configJson;
}
String getUpdateUrl(){
 return "http://release.bhonofre.pt/release_"+String(FACTORY_TYPE)+".bin";
 }
String getHostname() {
    String nodeId = configJson.get<String>("nodeId");
    if (nodeId.equals(configJson.get<String>("hostname"))) {
        return nodeId;
    }
    return String(HARDWARE) + "-" + String(ESP.getChipId()) + "-" + nodeId;
}

String getApName() {

    String nodeId = configJson.get<String>("nodeId");
    if (nodeId.equals(configJson.get<String>("hostname"))) {
        return "bhnode-" + nodeId;
    }
    return "bhnode-" + String(ESP.getChipId());
}

void loadStoredConfiguration() {
    bool configFail = true;
    if (SPIFFS.begin()) {
        File cFile;
        if (SPIFFS.exists(CONFIG_FILENAME)) {
            cFile = SPIFFS.open(CONFIG_FILENAME, "r+");
            if (cFile) {
                logger("[CONFIG] Read stored file config...");
                JsonObject &storedConfig = getJsonObject(cFile);
                if (storedConfig.success()) {
                    configJson.set("nodeId", storedConfig.get<String>("nodeId"));
                    configJson.set("homeAssistantAutoDiscovery", storedConfig.get<bool>("homeAssistantAutoDiscovery"));
                    configJson.set("homeAssistantAutoDiscoveryPrefix",
                                   storedConfig.get<String>("homeAssistantAutoDiscoveryPrefix"));
                    configJson.set("hostname", storedConfig.get<String>("hostname"));
                    configJson.set("mqttIpDns", storedConfig.get<String>("mqttIpDns"));
                    configJson.set("mqttUsername", storedConfig.get<String>("mqttUsername"));
                    configJson.set("mqttPassword", storedConfig.get<String>("mqttPassword"));
                    configJson.set("wifiSSID", storedConfig.get<String>("wifiSSID"));
                    configJson.set("wifiSecret", storedConfig.get<String>("wifiSecret"));
                    configJson.set("wifiIp", storedConfig.get<String>("wifiIp"));
                    configJson.set("hardwareId", String(ESP.getChipId()));

                    configJson.set("notificationInterval", storedConfig.get<unsigned int>("notificationInterval"));
                    configJson.set("directionCurrentDetection", storedConfig.get<bool>("directionCurrentDetection"));
                    configJson.set("emoncmsApiKey", storedConfig.get<String>("emoncmsApiKey"));
                    configJson.set("emoncmsPrefix", storedConfig.get<String>("emoncmsPrefix"));
                    configJson.set("emoncmsUrl", storedConfig.get<String>("emoncmsUrl"));
                    configJson.set("emoncmsPort", storedConfig.get<int>("emoncmsPort"));
                    configJson.set("hardware", "PZEM");
                    configJson.set("type", String(FACTORY_TYPE));
                    configJson.set("wifiMask", storedConfig.get<String>("wifiMask"));
                    configJson.set("wifiGw", storedConfig.get<String>("wifiGw"));
                    configJson.set("staticIp", storedConfig.get<bool>("staticIp"));
                    configJson.set("apSecret", storedConfig.get<String>("apSecret"));
                    configJson.set("configTime", storedConfig.get<long>("configTime"));
                    double configVersion = storedConfig.get<double>("configVersion");
                    logger("[CONFIG] CONFIG UPDATED Version " + String(configVersion));
                    configJson.set("configVersion", FIRMWARE_VERSION);
                    logger("[CONFIG] Apply stored file config with success...");
                    cFile.close();
                    configFail = false;
                }
            }
        }

        if (configFail) {
            logger("[CONFIG] Apply default config...");
            cFile = SPIFFS.open(CONFIG_FILENAME, "w+");
            configJson.set("nodeId",String(HARDWARE) +"-"+String(FACTORY_TYPE)+"-"+String(ESP.getChipId()));
            configJson.set("homeAssistantAutoDiscovery", true);
            configJson.set("homeAssistantAutoDiscoveryPrefix","homeassistant");
            configJson.set("hostname",String(HARDWARE) +"-"+String(FACTORY_TYPE)+"-"+String(ESP.getChipId()));
            configJson.set("mqttIpDns", MQTT_BROKER_IP);
            configJson.set("mqttUsername", MQTT_USERNAME);
            configJson.set("mqttPassword", MQTT_PASSWORD);
            configJson.set("wifiSSID", WIFI_SSID);
            configJson.set("wifiSecret", WIFI_SECRET);
            configJson.set("configVersion", FIRMWARE_VERSION);
            configJson.set("apSecret", AP_SECRET);
            configJson.set("emoncmsPort", 80);
            configJson.set("type", String(FACTORY_TYPE));
            configJson.set("configTime",0L);
            configJson.set("directionCurrentDetection", false);
            configJson.set("notificationInterval", DELAY_NOTIFICATION);
            configJson.set("hardware", "PZEM");
            configJson.printTo(cFile);
        }
        SPIFFS.end();
    } else {
        logger("[CONFIG] File system error...");
    }
}


JsonObject &saveNode(JsonObject &nodeConfig) {
    String nodeId = nodeConfig.get<String>("nodeId");

    if (nodeId != nullptr && !configJson.get<String>("nodeId").equals(nodeId)) {
        nodeId.replace(" ", "");
        String oldNodeId = configJson.get<String>("nodeId");
        configJson.set("nodeId", nodeId);

        //reloadWiFiConfig();
        reloadMqttConfig();
        
    } else {

        configJson.set("notificationInterval", nodeConfig.get<unsigned int>("notificationInterval"));
        configJson.set("directionCurrentDetection", nodeConfig.get<bool>("directionCurrentDetection"));

    }
    saveConfig();
    return configJson;
}

JsonObject &saveWifi(JsonObject &_config) {
    configJson.set("wifiSSID", _config.get<String>("wifiSSID"));
    configJson.set("wifiSecret", _config.get<String>("wifiSecret"));
    configJson.set("wifiIp", _config.get<String>("wifiIp"));
    configJson.set("wifiMask", _config.get<String>("wifiMask"));
    configJson.set("wifiGw", _config.get<String>("wifiGw"));
    configJson.set("staticIp", _config.get<bool>("staticIp"));
    configJson.set("apSecret", _config.get<String>("apSecret"));
    wifiUpdated = true;
    return configJson;
}

JsonObject& adoptControllerConfig(JsonObject &_config) {
    configJson.set("wifiSSID", _config.get<String>("wifiSSID"));
    configJson.set("wifiSecret", _config.get<String>("wifiSecret"));
    configJson.set("apSecret", _config.get<String>("apSecret"));
    configJson.set("mqttIpDns", _config.get<String>("mqttIpDns"));
    configJson.set("mqttUsername", _config.get<String>("mqttUsername"));
    configJson.set("mqttPassword", _config.get<String>("mqttPassword"));
    configJson.set("mqttEmbedded", _config.get<String>("mqttEmbedded"));
    configJson.set("emoncmsApiKey", _config.get<String>("emoncmsApiKey"));
    configJson.set("emoncmsPrefix", _config.get<String>("emoncmsPrefix"));
    configJson.set("emoncmsUrl", _config.get<String>("emoncmsUrl"));
    configJson.set("emoncmsPort", _config.get<int>("emoncmsPort"));
      configJson.set("configTime",_config.get<long>("configTime"));
    adopted = true;
    return configJson;
}

void updateNetworkConfig() {
    if (!configJson.get<bool>("staticIp")) {
        configJson.set("wifiIp", WiFi.localIP().toString());
        configJson.set("wifiMask", WiFi.subnetMask().toString());
        configJson.set("wifiGw", WiFi.gatewayIP().toString());
    }
    saveConfig();
}

JsonObject &saveMqtt(JsonObject &_config) {
    configJson.set("mqttIpDns", _config.get<String>("mqttIpDns"));
    configJson.set("mqttUsername", _config.get<String>("mqttUsername"));
    configJson.set("mqttPassword", _config.get<String>("mqttPassword"));
    configJson.set("mqttEmbedded", _config.get<String>("mqttEmbedded"));
    saveConfig();
    _config.printTo(Serial);
    reloadMqttConfig();
    return configJson;
}

JsonObject &saveHa(JsonObject &_config) {
    configJson.set("homeAssistantAutoDiscovery", _config.get<bool>("homeAssistantAutoDiscovery"));
    configJson.set("homeAssistantAutoDiscoveryPrefix", _config.get<String>("homeAssistantAutoDiscoveryPrefix"));
    saveConfig();
    return configJson;
}


JsonObject &saveEmoncms(JsonObject &_config) {
    configJson.set("emoncmsApiKey", _config.get<String>("emoncmsApiKey"));
    configJson.set("emoncmsPrefix", _config.get<String>("emoncmsPrefix"));
    configJson.set("emoncmsUrl", _config.get<String>("emoncmsUrl"));
    configJson.set("emoncmsPort", _config.get<int>("emoncmsPort"));
    saveConfig();
    return configJson;
}

void saveConfig() {
    if (SPIFFS.begin()) {
        File rFile = SPIFFS.open(CONFIG_FILENAME, "w+");
        if (!rFile) {
            logger("[CONFIG] Open config file Error!");
        } else {

            configJson.printTo(rFile);
        }
        rFile.close();
    } else {
        logger("[CONFIG] Open file system Error!");
    }
    SPIFFS.end();
    logger("[CONFIG] New config stored.");

}

void releaseGpio(int gpio) {

}

void configGpio(int gpio, int mode) {
    pinMode(gpio, mode);
    inUseGpios.push_back({gpio});
}
