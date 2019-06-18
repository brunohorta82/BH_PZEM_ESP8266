#include "Templates.h"
#define RELAY_TYPE "relay"
#define MQTT_TYPE "mqtt"
#define SWITCH_DEVICE "switch"
#define BUTTON_SWITCH 1
#define BUTTON_PUSH 2
#define OPEN_CLOSE_SWITCH 4
#define OPEN_CLOSE_PUSH 5
#define AUTO_OFF 6
const String switchsFilename = "switchs.json";

String statesPool[] = {"OPEN", "STOP", "CLOSE", "STOP"};

JsonArray &sws = getJsonArray();

bool coverNeedsStop;
unsigned long openCloseonTime;
unsigned int _gpioStop;
unsigned int swsSize;
typedef struct
{
  DebounceEvent *debouncer;
} switch_t;
std::vector<switch_t> _switchs;

void callback(uint8_t gpio, uint8_t event, uint8_t count, uint16_t length)
{
  for (unsigned int i = 0; i < sws.size(); i++)
  {
    JsonObject &switchJson = sws.get<JsonVariant>(i);
    if (switchJson.get<unsigned int>("gpio") == gpio){
        if(switchJson.get<String>("type").equals("cover")){
          if (event == EVENT_RELEASED){
            int currentStatePool = switchJson.get<unsigned int>("currentStatePool");
            stateSwitch(switchJson, statesPool[currentStatePool % 4]);
            switchJson.set("currentStatePool", currentStatePool + 1);
          }
        }else if(switchJson.get<String>("type").equals("light") || switchJson.get<String>("type").equals("switch")){
        if (event == EVENT_CHANGED || event == EVENT_RELEASED){
          stateSwitch(switchJson, switchJson.get<bool>("stateControl") ? "OFF" : "ON");
        }
      }
    }
    else if (switchJson.get<unsigned int>("gpioOpen") == gpio)
    {
      if (event == EVENT_CHANGED)
      {
        if (!digitalRead(gpio))
        {
          stateSwitch(switchJson, PAYLOAD_OPEN);
        }
        else
        {
          if (digitalRead(switchJson.get<unsigned int>("gpioClose")))
          {
            stateSwitch(switchJson, PAYLOAD_STOP);
          }
        }
      }
    }
    else if (switchJson.get<unsigned int>("gpioClose") == gpio)
    {
      if (event == EVENT_CHANGED)
      {
        if (!digitalRead(gpio))
        {
          stateSwitch(switchJson, PAYLOAD_CLOSE);
        }
        else
        {

          if (digitalRead(switchJson.get<unsigned int>("gpioOpen")))
          {
            stateSwitch(switchJson, PAYLOAD_STOP);
          }
        }
      }
    }
  }
}

JsonObject &storeSwitch(JsonObject &_switch)
{
  removeSwitch(_switch.get<String>("id"));
 _switch.set("id", normalize(_switch.get<String>("name")));
 _switch.set("class", "switch");

  rebuildSwitchMqttTopics(_switch);
  rebuildDiscoverySwitchMqttTopics(_switch);
    String sw = "";
  _switch.printTo(sw);
  sws.add(getJsonObject(sw.c_str()));
  persistSwitchesFile();
  return _switch;
}

void coverAutoStop(int gpioStop)
{
  logger("[COVERS] AUTO STOP");
  coverNeedsStop = true;
  openCloseonTime = millis();
  _gpioStop = gpioStop;
}

void stateSwitch(JsonObject &switchJson, String state)
{

  if (switchJson.get<String>("typeControl").equals(RELAY_TYPE))
  {
    if (String(PAYLOAD_OPEN).equals(state))
    {
      openAction(switchJson);
    }
    else if (String(PAYLOAD_STOP).equals(state))
    {
      stopAction(switchJson);
    }
    else if (String(PAYLOAD_CLOSE).equals(state))
    {
      closeAction(switchJson);
    }
    else if (String(PAYLOAD_ON).equals(state))
    {
      turnOn(switchJson);
    }
    else if (String(PAYLOAD_OFF).equals(state))
    {
      turnOff(switchJson);
    }
    else if (String(PAYLOAD_LOCK).equals(state))
    {
      //TODO
    }
    else if (String(PAYLOAD_UNLOCK).equals(state))
    {
      //TODO
    }
  }
}

void applyJsonSwitchs()
{
  _switchs.clear();
  for (int i = 0; i < sws.size(); i++)
  {
    JsonObject &switchJson = sws.get<JsonVariant>(i);
    uint8_t mode = BUTTON_DEFAULT_HIGH;
    if (switchJson.get<unsigned int>("mode") == OPEN_CLOSE_SWITCH || switchJson.get<unsigned int>("mode") == BUTTON_SWITCH)
    {
      mode = mode | BUTTON_SWITCH;
    }
    else if (switchJson.get<unsigned int>("mode") == OPEN_CLOSE_PUSH || switchJson.get<unsigned int>("mode") == BUTTON_PUSH)
    {
      mode = mode | BUTTON_PUSHBUTTON;
    }

    if (switchJson.get<bool>("pullup"))
    {
      mode = mode | BUTTON_SET_PULLUP;
    }

    if (switchJson.get<unsigned int>("mode") == OPEN_CLOSE_SWITCH)
    {
      _switchs.push_back({new DebounceEvent(switchJson.get<unsigned int>("gpioOpen"), callback, mode)});
      _switchs.push_back({new DebounceEvent(switchJson.get<unsigned int>("gpioClose"), callback, mode)});
    }
    else
    {
      _switchs.push_back({new DebounceEvent(switchJson.get<unsigned int>("gpio"), callback, mode)});
    }
  }
  swsSize = _switchs.size();
}

void mqttSwitchControl(String topic, String payload)
{
  for (unsigned int i = 0; i < sws.size(); i++)
  {
    JsonObject &switchJson = sws.get<JsonVariant>(i);
    if (switchJson.get<String>("mqttCommandTopic").equals(topic))
    {
      stateSwitch(switchJson, payload);
    }
  }
}

void publishState(JsonObject &switchJson)
{
  persistSwitchesFile();
  String swtr = "";
  switchJson.printTo(swtr);
  publishOnEventSource("switch", swtr);
  if (switchJson.get<String>("type").equals("cover"))
  {
    publishOnMqtt(switchJson.get<String>("mqttStateTopic").c_str(), switchJson.get<String>("stateControlCover"), switchJson.get<bool>("mqttRetain"));
    publishOnMqtt(switchJson.get<String>("mqttPositionStateTopic").c_str(), switchJson.get<String>("positionControlCover"), switchJson.get<bool>("mqttRetain"));
  }
  else
  {
    publishOnMqtt(switchJson.get<String>("mqttStateTopic"), switchJson.get<String>("statePayload"), switchJson.get<bool>("mqttRetain"));
  }
}

JsonArray &getStoredSwitchs()
{
  return sws;
}

void loadStoredSwitchs()
{
  bool loadDefaults = false;
  if (SPIFFS.begin())
  {
    File cFile;

    if (SPIFFS.exists(switchsFilename))
    {
      cFile = SPIFFS.open(switchsFilename, "r+");
      if (!cFile)
      {
        logger("[SWITCH] Create file switchs Error!");
        return;
      }
      logger("[SWITCH] Read stored file config...");
      JsonArray &storedSwitchs = getJsonArray(cFile);
      for (int i = 0; i < storedSwitchs.size(); i++)
      {
        sws.add(storedSwitchs.get<JsonVariant>(i));
      }
      if (!storedSwitchs.success())
      {
        logger("[SWITCH] Json file parse Error!");
        loadDefaults = true;
      }
      else
      {
        logger("[SWITCH] Apply stored file config...");
        applyJsonSwitchs();
      }
    }
    else
    {
      loadDefaults = true;
    }
    cFile.close();
    if (loadDefaults)
    {
      logger("[SWITCH] Apply default config...");
      cFile = SPIFFS.open(switchsFilename, "w+");
      if(String(FACTORY_TYPE).equals("cover")){
        sws.add(getJsonObject(COVER_SWITCH));
      }else if(String(FACTORY_TYPE).equals("light")){
        sws.add(getJsonObject(LIGHT_ONE));
        sws.add(getJsonObject(LIGHT_TWO));
       }
      sws.printTo(cFile);
      applyJsonSwitchs();
      cFile.close();
    }
  }
  else
  {
    logger("[SWITCH] Open file system Error!");
  }
  SPIFFS.end();
}

void persistSwitchesFile()
{

  if (SPIFFS.begin())
  {
    logger("[SWITCH] Open " + switchsFilename);
    File rFile = SPIFFS.open(switchsFilename, "w+");
    if (!rFile)
    {
      logger("[SWITCH] Open switch file Error!");
    }
    else
    {

      sws.printTo(rFile);
    }
    rFile.close();
  }
  else
  {
    logger("[SWITCH] Open file system Error!");
  }
  SPIFFS.end();
  applyJsonSwitchs();
  logger("[SWITCH] New switch config loaded.");
}

void removeSwitch(String _id)
{
  int switchFound = false;
  int index = 0;
  for (unsigned int i = 0; i < sws.size(); i++)
  {
    JsonObject &switchJson = sws.get<JsonVariant>(i);
    if (switchJson.get<String>("id").equals(_id))
    {
      switchFound = true;
      index = i;
      removeFromAlexaDiscovery(switchJson.get<String>("name"));
      removeFromHaDiscovery(switchJson.get<String>("type"), switchJson.get<String>("id"));
    }
  }
  if (switchFound)
  {
    sws.remove(index);
  }

  persistSwitchesFile();
}

void loopSwitchs()
{
  for (unsigned int i = 0; i < swsSize; i++)
  {
    DebounceEvent *b = _switchs[i].debouncer;
    b->loop();
  }
}
