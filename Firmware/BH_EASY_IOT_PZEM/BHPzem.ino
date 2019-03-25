
//TEMPERATURA
#include <OneWire.h>

#define DIRECTION_PIN 14
#define PZEM_READINDS_TOPIC  "/readings/status"
#define RX_PIN 4
#define TX_PIN 5
#define DS18B20_PIN 12
#define MAX_ATTEMPTS 5
#define DELAY_NOTIFICATION 3000 //10 seconds


#ifdef PZEM004
#include <PZEM004T.h> //https://github.com/olehs/PZEM004T
IPAddress pzemIP(192, 168, 1, 1);
PZEM004T pzem(RX_PIN, TX_PIN);
#endif

#ifdef PZEMDC
#include <TasmotaModbus.h>
#define PZEM_DC_DEVICE_ADDRESS  0x01
TasmotaModbus *PzemDcModbus;
#endif


DeviceAddress sensores[8];


Timing timerRead;
//DALLAS
OneWire oneWire(DS18B20_PIN);
DallasTemperature sensors(&oneWire);
DeviceAddress devAddr[15];  // array of (up to) 15 temperature sensors
String devAddrNames[15];  // array of (up to) 15 temperature sensors
int sensorsCount = 0;
bool pzemError = false;
int pzemErrorAttemps = 0;
bool pzemrdy = false;
JsonObject& readingsJson = getJsonObject();

float requestTemperature(DeviceAddress deviceAddress) {
    float temp = 0;
    do {
        temp = sensors.getTempC(deviceAddress);
    } while (temp == 85.0 || temp == (-127.0));
    return temp;
}


int directionSignal() {
    if (getConfigJson().get<bool>("directionCurrentDetection"))
        return digitalRead(DIRECTION_PIN) ? -1 : 1;
    return 1;
}

#ifdef PZEM004
float getVoltage() {
    int i = 0;
    float r = -1.0;
    do {
        r = pzem.voltage(pzemIP);
        i++;
    } while (i < MAX_ATTEMPTS && r < 0.0);
    return r;
}

float getCurrent() {
    int i = 0;
    float r = -1.0;
    do {
        r = pzem.current(pzemIP);
        i++;
    } while (i < MAX_ATTEMPTS && r < 0.0);
    return r;
}

float getPower() {
    int i = 0;
    float r = -1.0;
    do {
        r = pzem.power(pzemIP);
        i++;
    } while (i < MAX_ATTEMPTS && r < 0.0);
    return r;
}

float getEnergy() {
    int i = 0;
    float r = -1.0;
    do {
        r = pzem.energy(pzemIP);
        i++;
    } while (i < MAX_ATTEMPTS && r < 0.0);
    return r;
}

#endif

void setupBHPzem() {

#ifdef PZEM004
    pzemrdy = pzem.setAddress(pzemIP);
#endif

#ifdef PZEMDC
    PzemDcModbus = new TasmotaModbus(RX_PIN, TX_PIN);
    uint8_t result = PzemDcModbus->Begin(9600, 2);
#endif

    delay(1000);
    pinMode(DIRECTION_PIN, INPUT);
    sensors.begin();
    sensorsCount = sensors.getDeviceCount();
    oneWire.reset_search();
    for (int i = 0; i < sensorsCount; i++) {
        if (!oneWire.search(devAddr[i])) {
            logger("Unable to find temperature sensors address");
        }
    }
    for (int a = 0; a < sensorsCount; a++) {
        String addr = "";
        for (uint8_t i = 0; i < 8; i++) {
            if (devAddr[0][i] < 16) addr += "0";
            addr += String(devAddr[a][i], HEX);
        }
        devAddrNames[a] = addr;
    }
    timerRead.begin(0);
    readingsJson.set("voltagem", 0);
    readingsJson.set("amperagem", 0);
    readingsJson.set("potencia", 0);
    readingsJson.set("contador", 0);

}


JsonObject& getPzemReadings() {
    return readingsJson;
}

void loopBHPzem() {
    loopSwitchDisplay();
    if (timerRead.onTimeout(getConfigJson().get<unsigned int>("notificationInterval"))) {

#ifdef PZEM004
        float v = pzemError ? -1 : getVoltage();
        float i = pzemError ? -1 : getCurrent();
        float p = pzemError ? -1 : getPower() * directionSignal();
        float e = pzemError ? -1 : getEnergy() / 1000;
#endif

#ifdef PZEMDC
        float v = -1;
        float i = -1;
        float p = -1;
        float e = -1;
        static uint8_t send_retry = 0;
        bool data_ready = PzemDcModbus->ReceiveReady();
        if (data_ready) {
            uint8_t buffer[22];
            uint8_t error = PzemDcModbus->ReceiveBuffer(buffer, 8);
            if (error) {
                logger("ERROR PZEM " + String(error));
            } else {
                v = (float) ((buffer[3] << 8) + buffer[4]) / 100.0;
                i = (float) ((buffer[5] << 8) + buffer[6]) / 100.0;
                p = (float) ((buffer[9] << 24) + (buffer[10] << 16) + (buffer[7] << 8) + buffer[8]) / 10.0;
                e = (float) ((buffer[13] << 24) + (buffer[14] << 16) + (buffer[11] << 8) + buffer[12]);
                readingsJson.set("voltagem", v);
                readingsJson.set("amperagem", i);
                readingsJson.set("potencia", p);
                readingsJson.set("contador", e);
                publishData();
                printOnDisplay(v, i, p, e, "");
            }
        }
        if (0 == send_retry || data_ready) {
            send_retry = 5;
            PzemDcModbus->Send(PZEM_DC_DEVICE_ADDRESS, 0x04, 0, 8);

        } else {
            send_retry--;

        }
#endif

#ifdef PZEM004
        if (v < 0 && i < 0 && p < 0 && e <= 0) {
            pzemError = true;
            pzemErrorAttemps++;
            if (pzemErrorAttemps == 1) {
                logger("[PZEM] Check connections, can't get data.");
            }
            if (pzemErrorAttemps > 5) {
                logger("[PZEM] Retry get data...");
                pzemErrorAttemps = 0;
                pzemError = false;
            }

        }
        readingsJson.set("voltagem", v);
        readingsJson.set("amperagem", i);
        readingsJson.set("potencia", p);
        readingsJson.set("contador", e);
        sensors.requestTemperatures();
        String displayTemps = "";
        for (int a = 0; a < sensorsCount; a++) {
            float t = requestTemperature(devAddr[a]);
            displayTemps += "t" + String(a + 1) + ": " + ((int) t) + " ºC ";
            readingsJson.set("temp_" + devAddrNames[a], t);

        }
        publishData();
        printOnDisplay(v, i, p, e, displayTemps);
#endif
    }


}

void createPzemSensors() {
    publishOnMqttQueue("homeassistant/sensor/" + getConfigJson().get<String>("nodeId") + "/counter/config",
                       ("{\"name\": \"" + getConfigJson().get<String>("nodeId") + "_counter\", \"state_topic\": \"" +
                        (getConfigJson().get<String>("nodeId") + String(PZEM_READINDS_TOPIC)) +
                        "\", \"value_template\": \"{{ value_json.contador }}\", \"unit_of_measurement\": \"ºkWh\",\"icon\":\"mdi:power-socket-eu\"}"),
                       true);
    publishOnMqttQueue("homeassistant/sensor/" + getConfigJson().get<String>("nodeId") + "/voltage/config",
                       ("{\"name\": \"" + getConfigJson().get<String>("nodeId") + "_voltage\", \"state_topic\": \"" +
                        (getConfigJson().get<String>("nodeId") + String(PZEM_READINDS_TOPIC)) +
                        "\", \"value_template\": \"{{ value_json.voltagem }}\", \"unit_of_measurement\": \"V\",\"icon\":\"mdi:power-socket-eu\"}"),
                       true);
    publishOnMqttQueue("homeassistant/sensor/" + getConfigJson().get<String>("nodeId") + "/amperage/config",
                       ("{\"name\": \"" + getConfigJson().get<String>("nodeId") + "_amperage\", \"state_topic\": \"" +
                        (getConfigJson().get<String>("nodeId") + String(PZEM_READINDS_TOPIC)) +
                        "\", \"value_template\": \"{{ value_json.amperagem }}\", \"unit_of_measurement\": \"A\",\"icon\":\"mdi:power-socket-eu\"}"),
                       true);
    publishOnMqttQueue("homeassistant/sensor/" + getConfigJson().get<String>("nodeId") + "/power/config",
                       ("{\"name\": \"" + getConfigJson().get<String>("nodeId") + "_power\", \"state_topic\": \"" +
                        (getConfigJson().get<String>("nodeId") + String(PZEM_READINDS_TOPIC)) +
                        "\", \"value_template\": \"{{ value_json.potencia }}\", \"unit_of_measurement\": \"W\",\"icon\":\"mdi:power-socket-eu\"}"),
                       true);

}

void publishData() {
    publishOnEventSource("pzem-readings", readingsJson);
    //MQTT
    publishOnMqtt(getConfigJson().get<String>("nodeId") + String(PZEM_READINDS_TOPIC), readingsJson, false);
    //EMON CMS
    publishOnEmoncms(readingsJson);
}
