#include <dht_nonblocking.h> // https://github.com/brunohorta82/DHT_nonblocking
//FUNTIONS
#define TEMPERATURE_FUNCTION 1
#define HUMIDITY_FUNCTION 2
#define MOTION_FUNCTION 4
#define OPENING 5
#define ANALOG_FUNCTION 7

//SENSORS
#define PIR_TYPE 65
#define LDR_TYPE 21
#define DS18B20_TYPE 90
#define REED_SWITCH_TYPE 56

const String sensorsFilename = "sensors.json";

JsonArray &sns = getJsonArray();

typedef struct
{
  int type;
  String mqttTopicState;
  DHT_nonblocking *dht;
  DallasTemperature *dallas;
  DebounceEvent *binaryDebounce;
  int gpio;
  long delayRead;
  long lastRead;
  float temperature;
  float humidity;
  String payloadOn;
  String payloadOff;
} sensors_t;
std::vector<sensors_t> _sensors;
int _sensorsSize = 0;

void callbackBinarySensor(uint8_t gpio, uint8_t event, uint8_t count, uint16_t length)
{
  for (int i = 0; i < _sensorsSize; i++)
  {
    if(_sensors[i].gpio != gpio)continue;
    
    switch (_sensors[i].type)
    {
    case PIR_TYPE:
      if(event == EVENT_RELEASED){
          publishOnMqtt(_sensors[i].mqttTopicState.c_str(), _sensors[i].payloadOn.c_str(),false);
          
        }else if(event == EVENT_PRESSED){
          publishOnMqtt(_sensors[i].mqttTopicState.c_str(), _sensors[i].payloadOff.c_str(),false);
       }
      break;
    case REED_SWITCH_TYPE:
     if(event == EVENT_RELEASED){
          publishOnMqtt(_sensors[i].mqttTopicState.c_str(), _sensors[i].payloadOff.c_str(),false);
        }else if(event == EVENT_PRESSED){
          publishOnMqtt(_sensors[i].mqttTopicState.c_str(), _sensors[i].payloadOn.c_str(),false);
          
       }
      break;
    }
  }
}
void removeSensor(String _id)
{
  int sensorFound = false;
  int index = 0;
  for (unsigned int i = 0; i < sns.size(); i++)
  {
    JsonObject &sensorJson = sns.get<JsonVariant>(i);
    if (sensorJson.get<String>("id").equals(_id))
    {
      sensorFound = true;
      index = i;
        JsonArray &functions = sensorJson.get<JsonVariant>("functions");
    for (int i = 0; i < functions.size(); i++)
    {
      JsonObject &f = functions.get<JsonVariant>(i);
      String _id = normalize(f.get<String>("name"));
      removeFromHaDiscovery(sensorJson.get<String>("class"), _id);
    }
    }
  }
  if (sensorFound)
  {
    sns.remove(index);
  }

  persistSensorsFile();
}

JsonArray &getStoredSensors()
{
  return sns;
}

void loopSensors()
{
  for (unsigned int i = 0; i < _sensorsSize; i++)
  {
   
    switch (_sensors[i].type)
    {
    case LDR_TYPE:
      {
         if(_sensors[i].lastRead + _sensors[i].delayRead < millis()){
            logger("[SENSOR LDT] Reading...");
            _sensors[i].lastRead = millis();
            publishOnMqtt(_sensors[i].mqttTopicState.c_str(), String(analogRead(_sensors[i].gpio)).c_str(),false);
           
         }
      }
      break;
    case PIR_TYPE:
    {
      DebounceEvent *b = _sensors[i].binaryDebounce;
      b->loop();
    }
    break;
    case REED_SWITCH_TYPE:
    {
      DebounceEvent *b = _sensors[i].binaryDebounce;
      b->loop();
    }
    break;

    case DHT_TYPE_11:
    case DHT_TYPE_21:
    case DHT_TYPE_22:
    {

      if (_sensors[i].dht->measure(&_sensors[i].temperature, &_sensors[i].humidity) == true)
      {
      
      if(_sensors[i].lastRead + _sensors[i].delayRead < millis()){
            logger("[SENSOR DHT] Reading...");
            _sensors[i].lastRead = millis();
            publishOnMqtt(_sensors[i].mqttTopicState.c_str(), String("{\"temperature\":"+String(_sensors[i].temperature)+",\"humidity\":"+String(_sensors[i].humidity)+"}").c_str(),false);      
        }
      }
    }
      break;
    case DS18B20_TYPE:
    {
     
      if (_sensors[i].lastRead + _sensors[i].delayRead < millis())
      {
        logger("[SENSOR DALLAS] Reading...");
        _sensors[i].dallas->begin();
        _sensors[i].dallas->requestTemperatures();
        _sensors[i].lastRead = millis();
        _sensors[i].temperature = _sensors[i].dallas->getTempCByIndex(0);
        publishOnMqtt(_sensors[i].mqttTopicState.c_str(), String("{\"temperature\":"+String(_sensors[i].temperature)+"}").c_str(),false);
        
      }
    }
      break;
    }
  }
  
}
JsonObject &storeSensor(JsonObject &_sensor)
{
  removeSensor(_sensor.get<String>("id"));
  _sensor.set("id", normalize(_sensor.get<String>("name")));
  rebuildSensorMqttTopics(_sensor);
  rebuildDiscoverySensorMqttTopics(_sensor);
  String sn = "";
  _sensor.printTo(sn);
  sns.add(getJsonObject(sn.c_str()));

  persistSensorsFile();
  return _sensor;
}

void persistSensorsFile()
{

  if (SPIFFS.begin())
  {
    logger("[SENSORS] Open " + sensorsFilename);
    File rFile = SPIFFS.open(sensorsFilename, "w+");
    if (!rFile)
    {
      logger("[SENSORS] Open sensors file Error!");
    }
    else
    {
      sns.printTo(rFile);
    }
    rFile.close();
  }
  else
  {
    logger("[SENSORS] Open file system Error!");
  }
  SPIFFS.end();
  applyJsonSensors();
  logger("[SENSORS] New sensors config loaded.");
}
void loadStoredSensors()
{
  bool loadDefaults = false;
  if (SPIFFS.begin())
  {
    File cFile;

    if (SPIFFS.exists(sensorsFilename))
    {
      cFile = SPIFFS.open(sensorsFilename, "r+");
      if (!cFile)
      {
        logger("[SENSORS] Create file Sensor Error!");
        return;
      }
      logger("[SENSORS] Read stored file config...");
      JsonArray &storedSensors = getJsonArray(cFile);
      if (!storedSensors.success())
      {
        logger("[SENSORS] Json file parse Error!");
        loadDefaults = true;
      }
      else
      {

        logger("[SENSORS] Apply stored file config...");
        for (int i = 0; i < storedSensors.size(); i++)
        {
          sns.add(storedSensors.get<JsonVariant>(i));
        }
        applyJsonSensors();
      }
    }
    else
    {
      loadDefaults = true;
    }
    cFile.close();
    if (loadDefaults)
    {
      logger("[SENSORS] Apply default config...");
      cFile = SPIFFS.open(sensorsFilename, "w+");
      sns.printTo(cFile);
      applyJsonSensors();
      cFile.close();
    }
  }
  else
  {
    logger("[SWITCH] Open file system Error!");
  }
  SPIFFS.end();
}

void applyJsonSensors()
{
  _sensors.clear();

  for (int i = 0; i < sns.size(); i++)
  {
    JsonObject &sensorJson = sns.get<JsonVariant>(i);
    int gpio = sensorJson.get<unsigned int>("gpio");
    int type = sensorJson.get<unsigned int>("type");
    String mqttStateTopic = sensorJson.get<String>("mqttStateTopic");
     JsonArray &functions = sensorJson.get<JsonVariant>("functions");
    switch (type)
    {
    case LDR_TYPE:
      _sensors.push_back({type, mqttStateTopic, NULL, NULL, NULL, A0, 2000ul, 0ul, 0ul, 0ul,"",""});
      break;
    case PIR_TYPE:{
    JsonObject &f = functions.get<JsonVariant>(i);
    for (int i = 0; i < functions.size(); i++)
    {
      JsonObject &f = functions.get<JsonVariant>(i);
      String payloadOn = f.get<String>("payloadOn");
      String payloadOff = f.get<String>("payloadOff");
      _sensors.push_back({type, mqttStateTopic, NULL, NULL, new DebounceEvent(gpio, callbackBinarySensor, BUTTON_PUSHBUTTON | BUTTON_DEFAULT_HIGH), gpio, 0,0, 0, 0,payloadOn,payloadOff});
    }
    }
      break;
    case REED_SWITCH_TYPE:
    for (int i = 0; i < functions.size(); i++)
    {
      JsonObject &f = functions.get<JsonVariant>(i);
      String payloadOn = f.get<String>("payloadOn");
      String payloadOff = f.get<String>("payloadOff");
      int mode = BUTTON_PUSHBUTTON | BUTTON_SET_PULLUP ;
      if(gpio != 16){
        mode | BUTTON_DEFAULT_HIGH;
        }
      _sensors.push_back({type, mqttStateTopic, NULL, NULL, new DebounceEvent(gpio, callbackBinarySensor, mode), gpio, 0,0, 0, 0,payloadOn,payloadOff});
    }
      break;

    case DHT_TYPE_11:
    case DHT_TYPE_21:
    case DHT_TYPE_22:
      _sensors.push_back({type, mqttStateTopic, new DHT_nonblocking(gpio, type), NULL, NULL, gpio, 60000ul, 0L, 0L, 0L,"",""});
      break;
    case DS18B20_TYPE:
      _sensors.push_back({type, mqttStateTopic, NULL, new DallasTemperature(new OneWire(gpio)), NULL, gpio, 60000L, 0L, 0L, 0L,"",""});
      break;
    }
  }
  _sensorsSize = _sensors.size();
}
