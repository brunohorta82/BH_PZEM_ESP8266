
#define RELAY_DEVICE "relay"
#define NORMAL false
#define INVERTED true

JsonArray &rls = getJsonArray();

const String relaysFilename = "relays.json";

void removeRelay(String _id)
{
  int relayFound = false;
  int index = 0;
  for (unsigned int i = 0; i < rls.size(); i++)
  {
    JsonObject &relayJson = rls.get<JsonVariant>(i);
    if (relayJson.get<String>("id").equals(_id))
    {
      relayFound = true;
      index = i;
    }
  }
  if (relayFound)
  {
    rls.remove(index);
  }

  persistRelaysFile();
  applyJsonRelays();
}
JsonObject &saveRelay( JsonObject &_relay)
{
  removeRelay(_relay.get<String>("id"));
 _relay.set("id", normalize(_relay.get<String>("name")));
  String r = "";
  _relay.printTo(r);
  rls.add(getJsonObject(r.c_str()));
  persistRelaysFile();
  return _relay;
}
void openAction(JsonObject &switchJson)
{
  int gpioOpenClose = switchJson.get<unsigned int>("gpioControlOpenClose");
  int gpioStop = switchJson.get<unsigned int>("gpioControlStop");
  delay(10);
  _turnOff(getRelay(gpioStop));
  delay(50);
  logger("[SWITCH] OPEN");
  _turnOn(getRelay(gpioOpenClose));
  delay(50);
  _turnOn(getRelay(gpioStop));
  switchJson.set("positionControlCover", 100);
  switchJson.set("stateControlCover", "OPEN");
  switchJson.set("statePayload", "open");
  publishState(switchJson);
}

void closeAction(JsonObject &switchJson)
{
  int gpioOpenClose = switchJson.get<unsigned int>("gpioControlOpenClose");
  int gpioStop = switchJson.get<unsigned int>("gpioControlStop");
  delay(10);
  _turnOff(getRelay(gpioStop));
  delay(50);
  logger("[SWITCH] CLOSE");
  _turnOff(getRelay(gpioOpenClose));
  delay(50);
  _turnOn(getRelay(gpioStop));
  switchJson.set("positionControlCover", 0);
  switchJson.set("stateControlCover", "CLOSE");
  switchJson.set("statePayload", "closed");
  publishState(switchJson);
}

void stopAction(JsonObject &switchJson)
{
  int gpioOpenClose = switchJson.get<unsigned int>("gpioControlOpenClose");
  int gpioStop = switchJson.get<unsigned int>("gpioControlStop");
  logger("[SWITCH] STOP.");
  _turnOff(getRelay(gpioStop));
  switchJson.set("positionControlCover", 50);
  switchJson.set("stateControlCover", "STOP");
  switchJson.set("statePayload", "");

  publishState(switchJson);
}

void turnOn(JsonObject &switchJson)
{
  JsonObject &relay = getRelay(switchJson.get<unsigned int>("gpioControl"));
  bool state = _turnOn(relay);
  switchJson.set("stateControl", state);
  switchJson.set("statePayload", state ? "ON" : "OFF");
  publishState(switchJson);
}

void turnOff(JsonObject &switchJson)
{
  JsonObject &relay = getRelay(switchJson.get<unsigned int>("gpioControl"));
  bool state = _turnOff(relay);
  switchJson.set("stateControl", state);
  switchJson.set("statePayload", state ? "ON" : "OFF");
  publishState(switchJson);
}
bool _turnOn(JsonObject &relay)
{
  int gpio = relay.get<unsigned int>("gpio");
  bool inverted = relay.get<bool>("inverted");
  String name = relay.get<String>("name");
  digitalWrite(gpio, inverted ? LOW : HIGH);
  logger("[RELAY " + name + " GPIO: " + String(gpio) + "] ON");
  return inverted ? !digitalRead(gpio) : digitalRead(gpio);
}

bool _turnOff(JsonObject &relay)
{
  int gpio = relay.get<unsigned int>("gpio");
  bool inverted = relay.get<bool>("inverted");
  String name = relay.get<String>("name");
  digitalWrite(gpio, inverted ? HIGH : LOW);
  logger("[RELAY " + name + " GPIO: " + String(gpio) + "] OFF");
  return inverted ? !digitalRead(gpio) : digitalRead(gpio);
}
void initNormal(bool state, int gpio)
{
  if (state)
  {
    turnOn(getRelay(gpio));
  }
  else
  {
    turnOff(getRelay(gpio));
  }
}

bool toogleNormal(int gpio)
{
  if (digitalRead(gpio))
  {
    turnOff(getRelay(gpio));
  }
  else
  {
    turnOn(getRelay(gpio));
  }
  return digitalRead(gpio);
}
bool turnOnRelayNormal(int gpio)
{
  turnOn(getRelay(gpio));
  return digitalRead(gpio);
}
JsonObject &getRelay(int gpio)
{
  for (unsigned int i = 0; i < rls.size(); i++)
  {
    JsonObject &relayJson = rls.get<JsonVariant>(i);
    if (relayJson.get<unsigned int>("gpio") == gpio)
    {
      return relayJson;
    }
  }
  return getJsonObject();
}

JsonArray &getStoredRelays()
{
  return rls;
}

void loadStoredRelays()
{
  bool loadDefaults = false;
  if (SPIFFS.begin())
  {
    File cFile;

    if (SPIFFS.exists(relaysFilename))
    {
      cFile = SPIFFS.open(relaysFilename, "r+");
      if (!cFile)
      {
        logger("[RELAY] Create file relays Error!");
        return;
      }
      logger("[RELAY] Read stored file config...");
      JsonArray &storedRelays = getJsonArray(cFile);
      if (!storedRelays.success())
      {
        logger("[RELAY] Json file parse Error!");
        loadDefaults = true;
      }
      else
      {
        logger("[RELAY] Apply stored file config...");
        for (int i = 0; i < storedRelays.size(); i++)
        {
          rls.add(storedRelays.get<JsonVariant>(i));
        }
        applyJsonRelays();
      }
    }
    else
    {
      loadDefaults = true;
    }
    cFile.close();
    if (loadDefaults)
    {
      logger("[RELAY] Apply default config...");
      cFile = SPIFFS.open(relaysFilename, "w+");
      createDefaultRelays();
      rls.printTo(cFile);
      applyJsonRelays();
      cFile.close();
    }
  }
  else
  {
    logger("[RELAY] Open file system Error!");
  }
  SPIFFS.end();
}
void applyJsonRelays()
{
  for (int i = 0; i < rls.size(); i++)
  {
    JsonObject &relayJson = rls.get<JsonVariant>(i);
    int gpio = relayJson.get<unsigned int>("gpio");
    pinMode(gpio, OUTPUT);
  }
}
void persistRelaysFile()
{
  if (SPIFFS.begin())
  {
    logger("[RELAY] Open " + relaysFilename);
    File rFile = SPIFFS.open(relaysFilename, "w+");
    if (!rFile)
    {
      logger("[RELAY] Open relays file Error!");
    }
    else
    {

      rls.printTo(rFile);
    }
    rFile.close();
  }
  else
  {
    logger("[RELAY] Open file system Error!");
  }
  SPIFFS.end();
  logger("[RELAY] New relays config loaded.");
}
void relayJson(String _id, long _gpio, bool _inverted, String _name)
{
  JsonObject &relayJson = getJsonObject();
  relayJson.set("id", _id);
  relayJson.set("gpio", _gpio);
  relayJson.set("inverted", _inverted);
  relayJson.set("name", _name);
  relayJson.set("state", false);
  relayJson.set("class", RELAY_DEVICE);
  rls.add(relayJson);
}

void createDefaultRelays()
{
  relayJson("rele1", 4, NORMAL, "Relé 1"); // IN COVER IS STOP RELAY
  relayJson("rele2", 5, NORMAL, "Relé 2"); // IN COVER IS OPEN CLOSE RELAY
}
