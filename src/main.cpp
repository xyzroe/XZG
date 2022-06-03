#include <WiFi.h>

#include <WiFiClient.h>
#include <WebServer.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>
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

#include <ESPmDNS.h>

#include "mqtt.h"
#include <ESP32Ping.h>

#include <DNSServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;


// application config
unsigned long timeLog;
ConfigSettingsStruct ConfigSettings;
InfosStruct Infos;
bool configOK = false;
//String modeWiFi = "STA";

// serial end ethernet buffer size
#define BUFFER_SIZE 256

// multicast DNS responder
MDNSResponder mdns;

void mDNS_start();
bool setupSTAWifi();

void saveEmergencyWifi(bool state)
{
  const char *path = "/config/system.json";
  DynamicJsonDocument doc(1024);

  File configFile = LittleFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();

  doc["emergencyWifi"] = int(state);

  configFile = LittleFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
}

void saveBoard(int rev)
{
  const char *path = "/config/system.json";
  DynamicJsonDocument doc(1024);

  File configFile = LittleFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();

  doc["board"] = int(rev);

  configFile = LittleFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
}

bool checkPing()
{
  DEBUG_PRINT(F("Try to ping "));
  DEBUG_PRINTLN(ETH.gatewayIP());
  if (Ping.ping(ETH.gatewayIP()))
  {
    DEBUG_PRINTLN(F("okey ping"));
    //ConfigSettings.connectedEther = true;
    //ConfigSettings.disconnectEthTime = 0;
    if (ConfigSettings.emergencyWifi)
    {
      DEBUG_PRINTLN(F("saveEmergencyWifi(0)"));
      saveEmergencyWifi(0);
      DEBUG_PRINTLN(F("ESP.restart"));
      ESP.restart();
      //WiFi.disconnect();
      //WiFi.mode(WIFI_OFF);
      //ConfigSettings.emergencyWifi = 0;
    }
    return true;
  }
  else
  {
    DEBUG_PRINTLN(F("error ping"));
    return false;
    /*
    ConfigSettings.connectedEther = false;
    ConfigSettings.disconnectEthTime = millis();
    */
  }
  /*
  if (ConfigSettings.enableWiFi || ConfigSettings.emergencyWifi)
  {
    DEBUG_PRINT(F("ConfigSettings.enableWiFi "));
    DEBUG_PRINTLN(ConfigSettings.enableWiFi);
    DEBUG_PRINT(F("ConfigSettings.emergencyWifi "));
    DEBUG_PRINTLN(ConfigSettings.emergencyWifi);
    enableWifi();
  }
  */
}
void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_ETH_START:
    DEBUG_PRINTLN(F("ETH Started"));
    //ConfigSettings.disconnectEthTime = millis();
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    DEBUG_PRINTLN(F("ETH Connected"));
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
    if (checkPing())
    {
      ConfigSettings.connectedEther = true;
      ConfigSettings.disconnectEthTime = 0;
      mDNS_start();
    }
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
  case SYSTEM_EVENT_STA_DISCONNECTED:
    DEBUG_PRINTLN(F("WIFI STA DISCONNECTED")); 
    setupSTAWifi();
    break;
  default:
    break;
  }
}

WiFiServer server(TCP_LISTEN_PORT, MAX_SOCKET_CLIENTS);

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

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    float CPUtemp = getCPUtemp(true);
    int correct = CPUtemp - 30;
    String tempOffset = String(correct);

    String StringConfig = "{\"board\":1,\"emergencyWifi\":0,\"tempOffset\":" + tempOffset + "}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LittleFS.open(path, FILE_WRITE);
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
  ConfigSettings.tempOffset = (int)doc["tempOffset"];
  if (!ConfigSettings.tempOffset)
  {
    DEBUG_PRINTLN(F("no tempOffset in system.json"));
    configFile.close();

    float CPUtemp = getCPUtemp(true);
    int correct = CPUtemp - 30;
    String tempOffset = String(correct);
    doc["tempOffset"] = int(tempOffset.toInt());

    configFile = LittleFS.open(path, FILE_WRITE);
    serializeJson(doc, configFile);
    configFile.close();
    DEBUG_PRINTLN(F("saved tempOffset in system.json"));
    ConfigSettings.tempOffset = int(tempOffset.toInt());
  }
  configFile.close();
  return true;
}

bool loadConfigWifi()
{
  const char *path = "/config/configWifi.json";

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String StringConfig = "{\"enableWiFi\":0,\"ssid\":\"\",\"pass\":\"\",\"dhcpWiFi\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\",\"disableEmerg\":0}";

    writeDefultConfig(path, StringConfig);
  }

  configFile = LittleFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(path);
    return false;
  }

  ConfigSettings.dhcpWiFi = (int)doc["dhcpWiFi"];
  strlcpy(ConfigSettings.ssid, doc["ssid"] | "", sizeof(ConfigSettings.ssid));
  strlcpy(ConfigSettings.password, doc["pass"] | "", sizeof(ConfigSettings.password));
  strlcpy(ConfigSettings.ipAddressWiFi, doc["ip"] | "", sizeof(ConfigSettings.ipAddressWiFi));
  strlcpy(ConfigSettings.ipMaskWiFi, doc["mask"] | "", sizeof(ConfigSettings.ipMaskWiFi));
  strlcpy(ConfigSettings.ipGWWiFi, doc["gw"] | "", sizeof(ConfigSettings.ipGWWiFi));
  ConfigSettings.enableWiFi = (int)doc["enableWiFi"];
  ConfigSettings.disableEmerg = (int)doc["disableEmerg"];

  configFile.close();
  return true;
}

bool loadConfigEther()
{
  const char *path = "/config/configEther.json";

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String StringConfig = "{\"dhcp\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\"}";

    writeDefultConfig(path, StringConfig);
  }

  configFile = LittleFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(path);
    return false;
  }

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

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String deviceID = "ZigStarGW";
    //getDeviceID(deviceID);
    String StringConfig = "{\"hostname\":\"" + deviceID + "\",\"disableWeb\":0,\"refreshLogs\":1000,\"webAuth\":0,\"webUser\":"
                                                          ",\"webPass\":"
                                                          "}";

    writeDefultConfig(path, StringConfig);
  }

  configFile = LittleFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(path);
    return false;
  }

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

  ConfigSettings.webAuth = (int)doc["webAuth"];
  strlcpy(ConfigSettings.webUser, doc["webUser"] | "", sizeof(ConfigSettings.webUser));
  strlcpy(ConfigSettings.webPass, doc["webPass"] | "", sizeof(ConfigSettings.webPass));

  configFile.close();
  return true;
}

bool loadConfigSerial()
{
  const char *path = "/config/configSerial.json";

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String StringConfig = "{\"baud\":115200,\"port\":6638}";

    writeDefultConfig(path, StringConfig);
  }

  configFile = LittleFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(path);
    return false;
  }

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

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String deviceID;
    getDeviceID(deviceID);
    String StringConfig = "{\"enable\":0,\"server\":\"\",\"port\":1883,\"user\":\"mqttuser\",\"pass\":\"\",\"topic\":\"" + deviceID + "\",\"interval\":60,\"discovery\":0}";

    writeDefultConfig(path, StringConfig);
  }

  configFile = LittleFS.open(path, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(path);
    return false;
  }

  ConfigSettings.mqttEnable = (int)doc["enable"];
  strlcpy(ConfigSettings.mqttServer, doc["server"] | "", sizeof(ConfigSettings.mqttServer));
  ConfigSettings.mqttServerIP = parse_ip_address(ConfigSettings.mqttServer);
  ConfigSettings.mqttPort = (int)doc["port"];
  strlcpy(ConfigSettings.mqttUser, doc["user"] | "", sizeof(ConfigSettings.mqttUser));
  strlcpy(ConfigSettings.mqttPass, doc["pass"] | "", sizeof(ConfigSettings.mqttPass));
  strlcpy(ConfigSettings.mqttTopic, doc["topic"] | "", sizeof(ConfigSettings.mqttTopic));
  ConfigSettings.mqttInterval = (int)doc["interval"];
  ConfigSettings.mqttDiscovery = (int)doc["discovery"];

  configFile.close();
  return true;
}

void setupWifiAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.disconnect();

  String AP_NameString;
  getDeviceID(AP_NameString);

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);
  /*
  String WIFIPASSSTR = "ZigStar1";
  char WIFIPASS[WIFIPASSSTR.length() + 1];
  memset(WIFIPASS, 0, WIFIPASSSTR.length() + 1);
  for (int i = 0; i < WIFIPASSSTR.length(); i++)
    WIFIPASS[i] = WIFIPASSSTR.charAt(i);
*/
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_NameChar); //, WIFIPASS);
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);
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
      //modeWiFi = "AP";
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
    //modeWiFi = "AP";
    DEBUG_PRINTLN(F("AP"));
    ConfigSettings.radioModeWiFi = true;
  }
  mDNS_start();
}

void setupEthernetAndZigbeeSerial()
{
  switch (ConfigSettings.board)
  {

  case 1:
    if (ETH.begin(ETH_ADDR_1, ETH_POWER_PIN_1, ETH_MDC_PIN_1, ETH_MDIO_PIN_1, ETH_TYPE_1, ETH_CLK_MODE_1))
    {
      String boardName = "WT32-ETH01";
      boardName.toCharArray(ConfigSettings.boardName, sizeof(ConfigSettings.boardName));
      DEBUG_PRINT(F("Board - "));
      DEBUG_PRINTLN(boardName);
      ConfigSettings.rstZigbeePin = RESTART_ZIGBEE_1;
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
      String boardName = "TTGO T-Internet-POE";
      boardName.toCharArray(ConfigSettings.boardName, sizeof(ConfigSettings.boardName));
      DEBUG_PRINT(F("Board - "));
      DEBUG_PRINTLN(boardName);
      ConfigSettings.rstZigbeePin = RESTART_ZIGBEE_2;
      ConfigSettings.flashZigbeePin = FLASH_ZIGBEE_2;

      DEBUG_PRINT(F("Zigbee serial setup @ "));
      DEBUG_PRINTLN(ConfigSettings.serialSpeed);
      Serial2.begin(ConfigSettings.serialSpeed, SERIAL_8N1, ZRXD_2, ZTXD_2);

      oneWireBegin();
    }
    else
    {
      saveBoard(3);
      ESP.restart();
    }
    break;

  case 3:
    if (ETH.begin(ETH_ADDR_3, ETH_POWER_PIN_3, ETH_MDC_PIN_3, ETH_MDIO_PIN_3, ETH_TYPE_3, ETH_CLK_MODE_3))
    {
      String boardName = "unofficial China-GW";
      boardName.toCharArray(ConfigSettings.boardName, sizeof(ConfigSettings.boardName));
      DEBUG_PRINT(F("Board - "));
      DEBUG_PRINTLN(boardName);
      ConfigSettings.rstZigbeePin = RESTART_ZIGBEE_3;
      ConfigSettings.flashZigbeePin = FLASH_ZIGBEE_3;

      DEBUG_PRINT(F("Zigbee serial setup @ "));
      DEBUG_PRINTLN(ConfigSettings.serialSpeed);
      Serial2.begin(ConfigSettings.serialSpeed, SERIAL_8N1, ZRXD_3, ZTXD_3);
    }
    else
    {
      saveBoard(0);
      ESP.restart();
    }
    break;

  default:
    String boardName = "Unknown";
    if (!ETH.begin(ETH_ADDR_0, ETH_POWER_PIN_0, ETH_MDC_PIN_0, ETH_MDIO_PIN_0, ETH_TYPE_0, ETH_CLK_MODE_0))
    {
      ConfigSettings.emergencyWifi = 1;
      DEBUG_PRINTLN(F("Please set board type in system.json"));
      saveBoard(0);
    }
    else
    {
      saveBoard(1);
      boardName = "WT32-ETH01";
    }
    boardName.toCharArray(ConfigSettings.boardName, sizeof(ConfigSettings.boardName));
    DEBUG_PRINT(F("Board - "));
    DEBUG_PRINTLN(boardName);
    ConfigSettings.rstZigbeePin = RESTART_ZIGBEE_0;
    ConfigSettings.flashZigbeePin = FLASH_ZIGBEE_0;

    DEBUG_PRINT(F("Zigbee serial setup @ "));
    DEBUG_PRINTLN(ConfigSettings.serialSpeed);
    Serial2.begin(ConfigSettings.serialSpeed, SERIAL_8N1, ZRXD_0, ZTXD_0);
    break;
  }
}

void mDNS_start()
{
  if (!MDNS.begin(ConfigSettings.hostname))
  {
    DEBUG_PRINTLN(F("Error setting up MDNS responder!"));
    while (1)
    {
      delay(1000);
    }
  }
  else
  {
    DEBUG_PRINTLN(F("mDNS responder started"));
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("zigstar_gw", "tcp", ConfigSettings.socketPort);
    MDNS.addServiceTxt("zigstar_gw", "tcp", "version", "1.0");
    MDNS.addServiceTxt("zigstar_gw", "tcp", "radio_type", "znp");
    MDNS.addServiceTxt("zigstar_gw", "tcp", "baud_rate", String(ConfigSettings.serialSpeed));
    MDNS.addServiceTxt("zigstar_gw", "tcp", "data_flow_control", "software");
  }
}

void setup(void)
{

  Serial.begin(115200);
  DEBUG_PRINTLN(F("Start"));

  WiFi.onEvent(WiFiEvent);

  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
  {
    DEBUG_PRINTLN(F("Error with LITTLEFS"));
    return;
  }

  DEBUG_PRINTLN(F("LITTLEFS OK"));
  if (!loadSystemVar())
  {
    DEBUG_PRINTLN(F("Error load system vars"));
    const char *path = "/config";

    if (LittleFS.mkdir(path))
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
    DEBUG_PRINTLN(F("System vars load OK"));
  }

  if (!loadConfigSerial())
  {
    DEBUG_PRINTLN(F("Error load config serial"));
    ESP.restart();
  }
  else
  {
    DEBUG_PRINTLN(F("Config serial load OK"));
  }

  setupEthernetAndZigbeeSerial();

  if ((!loadConfigWifi()) || (!loadConfigEther()) || (!loadConfigGeneral()) || (!loadConfigMqtt()))
  {
    DEBUG_PRINTLN(F("Error load config files"));
    ESP.restart();
  }
  else
  {
    configOK = true;
    DEBUG_PRINTLN(F("Config files load OK"));
  }

  /*
  String boardName;
  switch (ConfigSettings.board)
  {
  case 0:
    boardName = "Unknown";
    break;
  case 1:
    boardName = "WT32-ETH01";
    break;
  case 2:
    boardName = "TTGO T-Internet-POE";
    break;
  }
  boardName.toCharArray(ConfigSettings.boardName, sizeof(ConfigSettings.boardName));
*/

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

  /*
  DEBUG_PRINT(F("Try to ping "));
  DEBUG_PRINTLN(ETH.gatewayIP());
  if(Ping.ping(ETH.gatewayIP())) 
  {
    DEBUG_PRINTLN(F("okey ping"));
    ConfigSettings.connectedEther = true;
    ConfigSettings.disconnectEthTime = 0;
    if (ConfigSettings.emergencyWifi && !ConfigSettings.enableWiFi)
    {
      saveEmergencyWifi(0);
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      ConfigSettings.emergencyWifi = 0;
      DEBUG_PRINTLN(F("saveEmergencyWifi(0)"));
    }
  } 
  else 
  {
    DEBUG_PRINTLN(F("error ping"));
    ConfigSettings.connectedEther = false;
    ConfigSettings.disconnectEthTime = millis();
  }
  */
  if (ConfigSettings.enableWiFi || ConfigSettings.emergencyWifi)
  {
    DEBUG_PRINT(F("ConfigSettings.enableWiFi "));
    DEBUG_PRINTLN(ConfigSettings.enableWiFi);
    DEBUG_PRINT(F("ConfigSettings.emergencyWifi "));
    DEBUG_PRINTLN(ConfigSettings.emergencyWifi);
    enableWifi();
  }

  server.begin(ConfigSettings.socketPort);
  server.setNoDelay(true);

  ConfigSettings.connectedClients = 0;

  if (ConfigSettings.mqttEnable)
  {
    mqttConnectSetup();
  }

}

WiFiClient client[10];
//double loopCount;

void socketClientConnected(int client)
{
  if (ConfigSettings.connectedSocket[client] != true)
  {
    DEBUG_PRINT(F("Connected client "));
    DEBUG_PRINTLN(client);
    if (ConfigSettings.connectedClients == 0)
    {
      ConfigSettings.socketTime = millis();
      DEBUG_PRINT(F("Socket time "));
      DEBUG_PRINTLN(ConfigSettings.socketTime);
      mqttPublishIo("socket", "ON");
    }
    ConfigSettings.connectedSocket[client] = true;
    ConfigSettings.connectedClients++;
  }
}

void socketClientDisconnected(int client)
{
  if (ConfigSettings.connectedSocket[client] != false)
  {
    DEBUG_PRINT(F("Disconnected client "));
    DEBUG_PRINTLN(client);
    ConfigSettings.connectedSocket[client] = false;
    ConfigSettings.connectedClients--;
    if (ConfigSettings.connectedClients == 0)
    {
      ConfigSettings.socketTime = millis();
      DEBUG_PRINT(F("Socket time "));
      DEBUG_PRINTLN(ConfigSettings.socketTime);
      mqttPublishIo("socket", "OFF");
    }
  }
}

void printRecvSocket(size_t bytes_read, uint8_t net_buf[BUFFER_SIZE])
{
  char output_sprintf[2];
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
}

void printSendSocket(size_t bytes_read, uint8_t serial_buf[BUFFER_SIZE])
{
  char output_sprintf[2];
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
}

void loop(void)
{
  uint16_t net_bytes_read = 0;
  uint8_t net_buf[BUFFER_SIZE];

  uint16_t serial_bytes_read = 0;
  uint8_t serial_buf[BUFFER_SIZE];

  if (!ConfigSettings.disableWeb)
  {
    webServerHandleClient();
  }
  else
  {
    if (ConfigSettings.connectedClients == 0)
    {
      webServerHandleClient();
    }
  }

  if (ConfigSettings.connectedEther == 0 && ConfigSettings.disconnectEthTime != 0 && ConfigSettings.enableWiFi != 1 && ConfigSettings.emergencyWifi != 1 && ConfigSettings.disableEmerg == 0)
  {
    if ((millis() - ConfigSettings.disconnectEthTime) >= (ETH_ERROR_TIME * 1000))
    {
      DEBUG_PRINTLN(F("saveEmergencyWifi(1)"));
      saveEmergencyWifi(1);
      DEBUG_PRINTLN(F("ESP.restart"));
      ESP.restart();
    }
  }

  if (server.hasClient())
  {
    for (byte i = 0; i < MAX_SOCKET_CLIENTS; i++)
    {
      //find free/disconnected spot
      if (!client[i] || !client[i].connected())
      {
        if (client[i])
        {
          client[i].stop();
        }
        client[i] = server.available();
        continue;
      }
    }
    //no free/disconnected spot so reject
    WiFiClient TempClient = server.available();
    TempClient.stop();
  }

  for (byte cln = 0; cln < MAX_SOCKET_CLIENTS; cln++)
  {
    if (client[cln])
    {
      socketClientConnected(cln);
      while (client[cln].available())
      { // read from LAN
        net_buf[net_bytes_read] = client[cln].read();
        if (net_bytes_read < BUFFER_SIZE - 1)
          net_bytes_read++;
      } // send to Zigbee
      Serial2.write(net_buf, net_bytes_read);
      // print to web console
      printRecvSocket(net_bytes_read, net_buf);
      net_bytes_read = 0;
    }
    else
    {
      socketClientDisconnected(cln);
    }
  }

  if (Serial2.available())
  {
    while (Serial2.available())
    { // read from Zigbee
      serial_buf[serial_bytes_read] = Serial2.read();
      if (serial_bytes_read < BUFFER_SIZE - 1)
        serial_bytes_read++;
    }
    // send to LAN
    for (byte cln = 0; cln < MAX_SOCKET_CLIENTS; cln++)
    {
      if (client[cln])
        client[cln].write(serial_buf, serial_bytes_read);
    }
    // print to web console
    printSendSocket(serial_bytes_read, serial_buf);
    serial_bytes_read = 0;
  }

  if (ConfigSettings.mqttEnable && (ConfigSettings.connectedEther || ConfigSettings.enableWiFi || ConfigSettings.emergencyWifi))
  {
    mqttLoop();
  }
  if (WiFi.getMode() == 2)
  {
    dnsServer.processNextRequest();
  }
}
