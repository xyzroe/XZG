#include <WiFi.h>

#include <WiFiClient.h>
#include <WebServer.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "LITTLEFS.h"
#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include <Update.h>
#include "version.h"

#include <driver/uart.h>
#include <lwip/ip_addr.h>

#include <ETH.h>
#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif

#ifdef BONJOUR_SUPPORT
#include <ESPmDNS.h>
#endif

#include "mqtt.h"

// application config
unsigned long timeLog;
ConfigSettingsStruct ConfigSettings;
InfosStruct Infos;
bool configOK = false;
String modeWiFi = "STA";

// serial end ethernet buffer size
#define BUFFER_SIZE 256

#ifdef BONJOUR_SUPPORT
// multicast DNS responder
MDNSResponder mdns;
#endif

void saveEmergencyWifi(bool state)
{
  const char *path = "/config/system.json";
  DynamicJsonDocument doc(1024);

  File configFile = LITTLEFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();

  doc["emergencyWifi"] = int(state);

  configFile = LITTLEFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
}

void saveBoard(int rev)
{
  const char *path = "/config/system.json";
  DynamicJsonDocument doc(1024);

  File configFile = LITTLEFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();

  doc["board"] = int(rev);

  configFile = LITTLEFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
}

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_ETH_START:
    DEBUG_PRINTLN(F("ETH Started"));
    //set eth hostname here
    //ETH.setHostname("esp32-ethernet");
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    DEBUG_PRINTLN(F("ETH Connected"));
    ConfigSettings.connectedEther = true;
    ConfigSettings.disconnectEthTime = 0;
    if (ConfigSettings.emergencyWifi && !ConfigSettings.enableWiFi)
    {
      saveEmergencyWifi(0);
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      ConfigSettings.emergencyWifi = 0;
      DEBUG_PRINTLN(F("saveEmergencyWifi 0"));
    }
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    DEBUG_PRINTLN(F("ETH MAC: "));
    DEBUG_PRINT(ETH.macAddress());
    DEBUG_PRINT(F(", IPv4: "));
    DEBUG_PRINT(ETH.localIP());
    if (ETH.fullDuplex())
    {
      DEBUG_PRINT(F(", FULL_DUPLEX"));
    }
    DEBUG_PRINT(F(", "));
    DEBUG_PRINT(ETH.linkSpeed());
    DEBUG_PRINTLN(F("Mbps"));
    ConfigSettings.connectedEther = true;
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    DEBUG_PRINTLN(F("ETH Disconnected"));
    ConfigSettings.connectedEther = false;
    ConfigSettings.disconnectEthTime = millis();
    break;
  case SYSTEM_EVENT_ETH_STOP:
    DEBUG_PRINTLN(F("ETH Stopped"));
    ConfigSettings.connectedEther = false;
    ConfigSettings.disconnectEthTime = millis();
    break;
  default:
    break;
  }
}

WiFiServer server(TCP_LISTEN_PORT);

IPAddress parse_ip_address(const char *str)
{
  IPAddress result;
  int index = 0;

  result[0] = 0;
  while (*str)
  {
    if (isdigit((unsigned char)*str))
    {
      result[index] *= 10;
      result[index] += *str - '0';
    }
    else
    {
      index++;
      if (index < 4)
      {
        result[index] = 0;
      }
    }
    str++;
  }

  return result;
}

bool loadSystemVar()
{
  const char *path = "/config/system.json";

  File configFile = LITTLEFS.open(path, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    String StringConfig = "{\"board\":1,\"emergencyWifi\":0}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed write"));
      return false;
    }
    else
    {
      serializeJson(doc, configFile);
    }
    return false;
  }

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, configFile);

  ConfigSettings.board = (int)doc["board"];
  ConfigSettings.emergencyWifi = (int)doc["emergencyWifi"];

  configFile.close();
  return true;
}

bool loadConfigWifi()
{
  const char *path = "/config/configWifi.json";

  File configFile = LITTLEFS.open(path, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    String StringConfig = "{\"enableWiFi\":0,\"ssid\":\"\",\"pass\":\"\",\"dhcpWiFi\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed write"));
      return false;
    }
    else
    {
      serializeJson(doc, configFile);
    }
    configFile.close();
  }

  configFile = LITTLEFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, configFile);

  ConfigSettings.dhcpWiFi = (int)doc["dhcpWiFi"];
  strlcpy(ConfigSettings.ssid, doc["ssid"] | "", sizeof(ConfigSettings.ssid));
  strlcpy(ConfigSettings.password, doc["pass"] | "", sizeof(ConfigSettings.password));
  strlcpy(ConfigSettings.ipAddressWiFi, doc["ip"] | "", sizeof(ConfigSettings.ipAddressWiFi));
  strlcpy(ConfigSettings.ipMaskWiFi, doc["mask"] | "", sizeof(ConfigSettings.ipMaskWiFi));
  strlcpy(ConfigSettings.ipGWWiFi, doc["gw"] | "", sizeof(ConfigSettings.ipGWWiFi));
  ConfigSettings.enableWiFi = (int)doc["enableWiFi"];

  configFile.close();
  return true;
}

bool loadConfigEther()
{
  const char *path = "/config/configEther.json";

  File configFile = LITTLEFS.open(path, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    String StringConfig = "{\"dhcp\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed write"));
      return false;
    }
    else
    {
      serializeJson(doc, configFile);
    }
    configFile.close();
  }

  configFile = LITTLEFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, configFile);

  // affectation des valeurs , si existe pas on place une valeur par defaut
  ConfigSettings.dhcp = (int)doc["dhcp"];
  strlcpy(ConfigSettings.ipAddress, doc["ip"] | "", sizeof(ConfigSettings.ipAddress));
  strlcpy(ConfigSettings.ipMask, doc["mask"] | "", sizeof(ConfigSettings.ipMask));
  strlcpy(ConfigSettings.ipGW, doc["gw"] | "", sizeof(ConfigSettings.ipGW));
  configFile.close();
  return true;
}

bool loadConfigGeneral()
{
  const char *path = "/config/configGeneral.json";

  File configFile = LITTLEFS.open(path, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    String StringConfig = "{\"hostname\":\"ZigStarGW\",\"disableWeb\":0,\"refreshLogs\":1000}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed write"));
      return false;
    }
    else
    {
      serializeJson(doc, configFile);
    }
    configFile.close();
  }

  configFile = LITTLEFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, configFile);

  ConfigSettings.disableWeb = (int)doc["disableWeb"];
  if ((double)doc["refreshLogs"] < 1000)
  {
    ConfigSettings.refreshLogs = 1000;
  }
  else
  {
    ConfigSettings.refreshLogs = (double)doc["refreshLogs"];
  }
  strlcpy(ConfigSettings.hostname, doc["hostname"] | "", sizeof(ConfigSettings.hostname));
  configFile.close();
  return true;
}

bool loadConfigSerial()
{
  const char *path = "/config/configSerial.json";

  File configFile = LITTLEFS.open(path, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    String StringConfig = "{\"baud\":115200,\"port\":4444}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed write"));
      return false;
    }
    else
    {
      serializeJson(doc, configFile);
    }
    configFile.close();
  }

  configFile = LITTLEFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, configFile);

  ConfigSettings.serialSpeed = (int)doc["baud"];
  ConfigSettings.socketPort = (int)doc["port"];
  if (ConfigSettings.socketPort == 0)
  {
    ConfigSettings.socketPort = TCP_LISTEN_PORT;
  }
  configFile.close();
  return true;
}

bool loadConfigMqtt()
{
  const char *path = "/config/configMqtt.json";

  File configFile = LITTLEFS.open(path, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    String StringConfig = "{\"enable\":0,\"server\":\"\",\"port\":1883,\"user\":\"\",\"pass\":\"\",\"topic\":\"ZigStarGW\",\"interval\":60,\"discovery\":0}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LITTLEFS.open(path, FILE_WRITE);
    if (!configFile)
    {
      DEBUG_PRINTLN(F("failed write"));
      return false;
    }
    else
    {
      serializeJson(doc, configFile);
    }
    configFile.close();
  }

  configFile = LITTLEFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, configFile);

  ConfigSettings.mqttEnable = (int)doc["enable"];
  strlcpy(ConfigSettings.mqttServer, doc["server"] | "", sizeof(ConfigSettings.mqttServer));
  ConfigSettings.mqttServerIP = parse_ip_address(ConfigSettings.mqttServer);
  ConfigSettings.mqttPort = (int)doc["port"];
  strlcpy(ConfigSettings.mqttUser, doc["user"] | "", sizeof(ConfigSettings.mqttUser));
  strlcpy(ConfigSettings.mqttPass, doc["pass"] | "", sizeof(ConfigSettings.mqttPass));
  strlcpy(ConfigSettings.mqttTopic, doc["topic"] | "", sizeof(ConfigSettings.mqttTopic));
  //ConfigSettings.mqttRetain = (int)doc["retain"];
  ConfigSettings.mqttInterval = (int)doc["interval"];
  ConfigSettings.mqttDiscovery = (int)doc["discovery"];

  configFile.close();
  return true;
}

void setupWifiAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.disconnect();

  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ZigStar-GW-" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  String WIFIPASSSTR = "ZigStar1";
  char WIFIPASS[WIFIPASSSTR.length() + 1];
  memset(WIFIPASS, 0, WIFIPASSSTR.length() + 1);
  for (int i = 0; i < WIFIPASSSTR.length(); i++)
    WIFIPASS[i] = WIFIPASSSTR.charAt(i);

  WiFi.softAP(AP_NameChar, WIFIPASS);
  WiFi.setSleep(false);
}

bool setupSTAWifi()
{

  WiFi.mode(WIFI_STA);
  DEBUG_PRINTLN(F("WiFi.mode(WIFI_STA)"));
  WiFi.disconnect();
  DEBUG_PRINTLN(F("disconnect"));
  delay(100);

  WiFi.begin(ConfigSettings.ssid, ConfigSettings.password);
  WiFi.setSleep(false);
  DEBUG_PRINTLN(F("WiFi.begin"));
  /*
  IPAddress ip;
  parse_ip_address(ip, ConfigSettings.ipAddressWiFi)
      IPAddress ip_address = ip;
  parse_ip_address(ip, ConfigSettings.ipAddressWiFi)
      IPAddress gateway_address = ip;
  parse_ip_address(ip, ConfigSettings.ipAddressWiFi)
      IPAddress netmask = ip;
      */

  IPAddress ip_address = parse_ip_address(ConfigSettings.ipAddressWiFi);
  IPAddress gateway_address = parse_ip_address(ConfigSettings.ipGWWiFi);
  IPAddress netmask = parse_ip_address(ConfigSettings.ipMaskWiFi);

  if (!ConfigSettings.dhcpWiFi)
  {
    WiFi.config(ip_address, gateway_address, netmask);
    DEBUG_PRINTLN(F("WiFi.config"));
  }
  else
  {
    DEBUG_PRINTLN(F("Try DHCP"));
  }

  int countDelay = 50;
  while (WiFi.status() != WL_CONNECTED)
  {
    //DEBUG_PRINT(F("."));
    DEBUG_PRINT(WiFi.status());
    countDelay--;
    if (countDelay == 0)
    {
      return false;
    }
    delay(250);
  }
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINTLN(WiFi.localIP());
  DEBUG_PRINTLN(WiFi.subnetMask());
  DEBUG_PRINTLN(WiFi.gatewayIP());
  return true;
}

void enableWifi()
{
  WiFi.setHostname(ConfigSettings.hostname);
  if ((strlen(ConfigSettings.ssid) != 0) && (strlen(ConfigSettings.password) != 0))
  {
    DEBUG_PRINTLN(F("Ok SSID & PASS"));
    if (!setupSTAWifi())
    {
      setupWifiAP();
      modeWiFi = "AP";
      ConfigSettings.radioModeWiFi = true;
      DEBUG_PRINTLN(F("AP"));
    }
    else
    {
      DEBUG_PRINTLN(F("setupSTAWifi"));
      ConfigSettings.radioModeWiFi = false;
    }
  }
  else
  {
    DEBUG_PRINTLN(F("Error SSID & PASS"));
    setupWifiAP();
    modeWiFi = "AP";
    DEBUG_PRINTLN(F("AP"));
    ConfigSettings.radioModeWiFi = true;
  }
}

void setup(void)
{

  Serial.begin(115200);
  DEBUG_PRINTLN(F("Start"));

  WiFi.onEvent(WiFiEvent);

  if (!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
  {
    DEBUG_PRINTLN(F("Error with LITTLEFS"));
    return;
  }

  DEBUG_PRINTLN(F("LITTLEFS OK"));
  if (!loadSystemVar())
  {
    DEBUG_PRINTLN(F("Error load system vars"));
    const char *path = "/config";

    if (LITTLEFS.mkdir(path))
    {
      DEBUG_PRINTLN(F("Config dir created"));
      delay(500);
      ESP.restart();
    }
    else
    {
      DEBUG_PRINTLN(F("mkdir failed"));
    }
  }
  else
  {
    //configOK = true;
    DEBUG_PRINTLN(F("System vars load OK"));
  }

  if ((!loadConfigWifi()) || (!loadConfigEther()) || (!loadConfigSerial()) || (!loadConfigGeneral()) || (!loadConfigMqtt()))
  {
    DEBUG_PRINTLN(F("Error load config files"));
    ESP.restart();
  }
  else
  {
    configOK = true;
    DEBUG_PRINTLN(F("Config files load OK"));
  }

  switch (ConfigSettings.board)
  {
  case 0:
    if (!ETH.begin(ETH_ADDR_1, ETH_POWER_PIN_1, ETH_MDC_PIN_1, ETH_MDIO_PIN_1, ETH_TYPE_1, ETH_CLK_MODE_1))
    {
      ConfigSettings.emergencyWifi = 1;
      DEBUG_PRINTLN(F("Unknown board. Please set type in system.json"));
    }
    else
    {
      DEBUG_PRINTLN(F("Looks like WT32-ETH01."));
      DEBUG_PRINTLN(F("Please set type in system.json"));
    }
    ConfigSettings.rstZigbeePin = RESET_ZIGBEE_1;
    ConfigSettings.flashZigbeePin = FLASH_ZIGBEE_1;

    DEBUG_PRINT(F("Zigbee serial setup @ "));
    DEBUG_PRINTLN(ConfigSettings.serialSpeed);
    Serial2.begin(ConfigSettings.serialSpeed, SERIAL_8N1, ZRXD_1, ZTXD_1);
    break;

  case 1:
    if (ETH.begin(ETH_ADDR_1, ETH_POWER_PIN_1, ETH_MDC_PIN_1, ETH_MDIO_PIN_1, ETH_TYPE_1, ETH_CLK_MODE_1))
    {
      DEBUG_PRINTLN(F("Board v1 (No POE) WT32-ETH01"));
      ConfigSettings.rstZigbeePin = RESET_ZIGBEE_1;
      ConfigSettings.flashZigbeePin = FLASH_ZIGBEE_1;

      DEBUG_PRINT(F("Zigbee serial setup @ "));
      DEBUG_PRINTLN(ConfigSettings.serialSpeed);
      Serial2.begin(ConfigSettings.serialSpeed, SERIAL_8N1, ZRXD_1, ZTXD_1);
    }
    else
    {
      saveBoard(2);
      ESP.restart();
    }
    break;

  case 2:
    if (ETH.begin(ETH_ADDR_2, ETH_POWER_PIN_2, ETH_MDC_PIN_2, ETH_MDIO_PIN_2, ETH_TYPE_2, ETH_CLK_MODE_2))
    {
      DEBUG_PRINTLN(F("Board v2 (with POE) TTGO T-Internet-POE"));
      ConfigSettings.rstZigbeePin = RESET_ZIGBEE_2;
      ConfigSettings.flashZigbeePin = FLASH_ZIGBEE_2;

      DEBUG_PRINT(F("Zigbee serial setup @ "));
      DEBUG_PRINTLN(ConfigSettings.serialSpeed);
      Serial2.begin(ConfigSettings.serialSpeed, SERIAL_8N1, ZRXD_2, ZTXD_2);
    }
    else
    {
      saveBoard(0);
      ESP.restart();
    }
    break;
  }

  //Config GPIOs
  pinMode(ConfigSettings.rstZigbeePin, OUTPUT);
  pinMode(ConfigSettings.flashZigbeePin, OUTPUT);
  digitalWrite(ConfigSettings.rstZigbeePin, 1);
  digitalWrite(ConfigSettings.flashZigbeePin, 1);

  ConfigSettings.disconnectEthTime = millis();
  ETH.setHostname(ConfigSettings.hostname);

  if (!ConfigSettings.dhcp)
  {
    DEBUG_PRINTLN(F("ETH STATIC"));
    ETH.config(parse_ip_address(ConfigSettings.ipAddress), parse_ip_address(ConfigSettings.ipGW), parse_ip_address(ConfigSettings.ipMask));
  }
  else
  {
    DEBUG_PRINTLN(F("ETH DHCP"));
  }

  initWebServer();

#ifdef BONJOUR_SUPPORT
  if (!MDNS.begin(ConfigSettings.hostname))
  {
    DEBUG_PRINTLN(F("Error setting up MDNS responder!"));
    while (1)
    {
      delay(1000);
    }
  }
  DEBUG_PRINTLN(F("mDNS responder started"));
  MDNS.addService("http", "tcp", 80);
#endif

  if (ConfigSettings.enableWiFi || ConfigSettings.emergencyWifi)
  {
    enableWifi();
  }
  server.begin(ConfigSettings.socketPort);

  //GetVersion
  //uint8_t cmdVersion[10] = {0x01, 0x02, 0x10, 0x10, 0x02, 0x10, 0x02, 0x10, 0x10, 0x03};
  //Serial2.write(cmdVersion, 10);
  if (ConfigSettings.mqttEnable)
  {
    mqttConnectSetup();
  }
}

String hexToDec(String hexString)
{

  unsigned int decValue = 0;
  int nextInt;

  for (int i = 0; i < hexString.length(); i++)
  {

    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57)
      nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70)
      nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102)
      nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);

    decValue = (decValue * 16) + nextInt;
  }

  return String(decValue);
}

WiFiClient client;
double loopCount;

void loop(void)
{
  size_t bytes_read;
  uint8_t net_buf[BUFFER_SIZE];
  uint8_t serial_buf[BUFFER_SIZE];

  if (!ConfigSettings.disableWeb)
  {
    webServerHandleClient();
  }
  else
  {
    if (!client.connected())
    {
      webServerHandleClient();
    }
  }

  if (ConfigSettings.connectedEther == false && ConfigSettings.disconnectEthTime != 0 && ConfigSettings.enableWiFi != 1 && ConfigSettings.emergencyWifi != 1)
  {
    if ((millis() - ConfigSettings.disconnectEthTime) >= (ETH_ERROR_TIME * 1000))
    {
      DEBUG_PRINTLN(F("saveEmergencyWifi(1)"));
      saveEmergencyWifi(1);
      DEBUG_PRINTLN(F("ESP.restart"));
      ESP.restart();
    }
  }
  // Check if a client has connected
  if (!client)
  {
    // eat any bytes in the swSer buffer as there is nothing to see them
    while (Serial2.available())
    {
      Serial2.read();
    }

    client = server.available();
  }
#define min(a, b) ((a) < (b) ? (a) : (b))
  char output_sprintf[2];
  if (client.connected())
  {
    if (ConfigSettings.connectedSocket != true)
    {
      ConfigSettings.connectedSocket = true;
      ConfigSettings.socketTime = millis();
      DEBUG_PRINT("true ConfigSettings.socketTime ");
      DEBUG_PRINTLN(ConfigSettings.socketTime);
      mqttPublishIo("socket", "ON");
    }
    int count = client.available();

    if (count > 0)
    {
      //DEBUG_PRINT("Buff count : ");
      //DEBUG_PRINTLN(count);
      bytes_read = client.read(net_buf, min(count, BUFFER_SIZE));
      Serial2.write(net_buf, bytes_read);

      if (bytes_read > 0)
      {
        String tmpTime;
        String buff = "";
        timeLog = millis();
        tmpTime = String(timeLog, DEC);
        logPush('[');
        for (int j = 0; j < tmpTime.length(); j++)
        {
          logPush(tmpTime[j]);
        }
        logPush(']');
        logPush('-');
        logPush('>');

        for (int i = 0; i < bytes_read; i++)
        {
          sprintf(output_sprintf, "%02x", net_buf[i]);
          logPush(' ');
          logPush(output_sprintf[0]);
          logPush(output_sprintf[1]);
        }
        logPush('\n');
      }
      Serial2.flush();
    }
  }
  else
  {
    client.stop();
    if (ConfigSettings.connectedSocket != false)
    {
      ConfigSettings.connectedSocket = false;
      ConfigSettings.socketTime = millis();
      DEBUG_PRINT("false ConfigSettings.socketTime ");
      DEBUG_PRINTLN(ConfigSettings.socketTime);
      mqttPublishIo("socket", "OFF");
    }
  }
  // now check the swSer for any bytes to send to the network
  bytes_read = 0;
  bool buffOK = false;

  while (Serial2.available() && bytes_read < BUFFER_SIZE)
  {
    buffOK = true;
    serial_buf[bytes_read] = Serial2.read();
    bytes_read++;
  }

  if (buffOK)
  {

    // uint8_t tmp[128];
    for (int i = 0; i < bytes_read; i++)
    {
      sprintf(output_sprintf, "%02x", serial_buf[i]);
      if (serial_buf[i] == 0x01)
      {

        String tmpTime;
        String buff = "";
        timeLog = millis();
        tmpTime = String(timeLog, DEC);
        logPush('[');
        for (int j = 0; j < tmpTime.length(); j++)
        {
          logPush(tmpTime[j]);
        }
        logPush(']');
        logPush('<');
        logPush('-');
      }
      logPush(' ');

      logPush(output_sprintf[0]);
      logPush(output_sprintf[1]);
      if (serial_buf[i] == 0x03)
      {
        logPush('\n');
      }
    }
    loopCount = 0;
    buffOK = false;
  }

  if (bytes_read > 0)
  {
    //DEBUG_PRINT("bytes_read : ");
    //DEBUG_PRINTLN(bytes_read);
    client.write((const uint8_t *)serial_buf, bytes_read);
    client.flush();
  }
  loopCount++;
  if (ConfigSettings.mqttEnable)
  {
    mqttLoop();
  }
}
