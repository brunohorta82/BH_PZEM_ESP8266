
#define FIRMWARE_VERSION 1.4
#define CONFIG_VERSION 2 //alterar para outro valor para rescrever as alterações
#define HOSTNAME "bhpzem-mynode"
#define NODE_ID "mynode"
#define MAX_ATTEMPTS 5
#define DELAY_NOTIFICATION 5000 //5 seconds
#define TEMPERATURE_PRECISION 9

#define WIFI_SSID "VOID SOFTWARE"
#define WIFI_SECRET "blackiscool"
//     ___ ___ ___ ___  _
//    / __| _ \_ _/ _ \( )___
//   | (_ |  _/| | (_) |/(_-<
//    \___|_| |___\___/  /__/
//
#define DIRECTION_PIN 14
#define RX_PIN 4
#define TX_PIN 5
#define DS18B20_PIN 12

//    ___ ___ ___ _____ _   _ ___ ___ ___
//   | __| __/ __|_   _| | | | _ \ __/ __|
//   | _|| _| (__  | | | |_| |   / _|\__ \
//   |_| |___\___| |_|  \___/|_|_\___|___/
//
#define DETECT_DIRECTION false
#define PRINT_TO_SERIAL_MONITOR false

//    ___  ___  ___ _____ _
//   | _ \/ _ \| _ \_   _( )___
//   |  _/ (_) |   / | | |/(_-<
//   |_|  \___/|_|_\ |_|   /__/
//
#define HTTPS_PORT 443
#define HTTP_PORT 80

//    ___ __  __  ___  _  _  ___ _  _ ___
//   | __|  \/  |/ _ \| \| |/ __| \| / __|
//   | _|| |\/| | (_) | .` | (__| .` \__ \
//   |___|_|  |_|\___/|_|\_|\___|_|\_|___/
//
#define EMONCMS_PROTOCOL 0 //usar 0 para HTTP ou 1 HTTPS para servidores com certificado
#define EMONCMS_HOST ""
#define EMONCMS_URL_PREFIX ""
#define EMONCMS_API_KEY ""

//    __  __  ___ _____ _____
//   |  \/  |/ _ \_   _|_   _|
//   | |\/| | (_) || |   | |
//   |_|  |_|\__\_\|_|   |_|
//
#define MQTT_BROKER_IP "192.168.187.227"
#define MQTT_BROKER_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_TOPIC "/bhpzem/" + String(NODE_ID) + "/readings/status"

//    ___ ___ ___ ___ _      ___   ___
//   |   \_ _/ __| _ \ |    /_\ \ / ( )___
//   | |) | |\__ \  _/ |__ / _ \ V /|/(_-<
//   |___/___|___/_| |____/_/ \_\_|   /__/
//
//#define D_SSD1306
String hostname = HOSTNAME;
String nodeId = NODE_ID;
int relayCount = 2;
int notificationInterval = DELAY_NOTIFICATION;
bool directionCurrentDetection = DETECT_DIRECTION;
String emoncmsApiKey = EMONCMS_API_KEY;
String emoncmsUrl = EMONCMS_HOST;
String emoncmsPrefix = EMONCMS_URL_PREFIX;
String mqttIpDns = MQTT_BROKER_IP;
String mqttUsername = MQTT_USERNAME;
String mqttPassword = MQTT_PASSWORD;

bool emoncmshttp = EMONCMS_PROTOCOL == 0;
String fileName = "/bconfig.json";
String wifiSSID = WIFI_SSID;
String wifiSecret = WIFI_SECRET;
int reservedGPIOS[] = {RX_PIN, TX_PIN, DIRECTION_PIN, DS18B20_PIN};

String IO_16 = "16|relay_2|INVERTED";
String IO_13 = "13|relay_1|NORMAL";
String IO_00 = "00|DISPLAY|SDA";
String IO_02 = "02|DISPLAY|SCL";
String IO_15 = "";
const int totalAvailableGPIOs = 5;
String availableGPIOS[] = {IO_16, IO_13, IO_00, IO_02, IO_15};
bool shouldReboot = false;
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;
    
    for (int i = 0; i <= maxIndex && found <= index; i++)
    {
        if (data.charAt(i) == separator || i == maxIndex)
        {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
