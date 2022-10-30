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
#include "Ticker.h"

#include <driver/uart.h>
#include <lwip/ip_addr.h>

#include <ETH.h>
#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif

#include <ESPmDNS.h>
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
volatile bool btnFlag = false;
bool btnLongPress = false;
//String modeWiFi = "STA";

// serial end ethernet buffer size
#define BUFFER_SIZE 256

// multicast DNS  responder
MDNSResponder mdns;

void mDNS_start();
bool setupSTAWifi();
void setupWifiAP();
void handlelongBtn();

Ticker tmrBtnLongPress(handlelongBtn, 1000, 0, MILLIS);


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
  if (ConfigSettings.disablePingCtrl == 1)
  {
    DEBUG_PRINTLN(F("Ping control disabled"));
    return true;
  }
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
  DEBUG_PRINT(F("WiFiEvent "));
  DEBUG_PRINTLN(event);
  switch (event)
  {
  case 18://SYSTEM_EVENT_ETH_START:
    DEBUG_PRINTLN(F("ETH Started"));
    //ConfigSettings.disconnectEthTime = millis();
    break;
  case 20://SYSTEM_EVENT_ETH_CONNECTED:
    DEBUG_PRINTLN(F("ETH Connected"));
    break;
  case 22://SYSTEM_EVENT_ETH_GOT_IP:
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
      //mDNS_start();
    }
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    DEBUG_PRINTLN(F("SYSTEM_EVENT_STA_GOT_IP"));
    ConfigSettings.wifiRetries = 0;
  case 21://SYSTEM_EVENT_ETH_DISCONNECTED:
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
    ConfigSettings.wifiRetries++;
    DEBUG_PRINT(ConfigSettings.wifiRetries); 
    DEBUG_PRINTLN(F(" times")); 
    if (ConfigSettings.wifiRetries < 7) {
      setupSTAWifi();
    }
    else 
    {
      setupWifiAP();
    }
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

  ConfigSettings.disableEmerg = 1;
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
  ConfigSettings.restarts = (int)doc["restarts"];
  configFile.close();
  return true;
}

bool loadConfigWifi()
{
  const char *path = "/config/configWifi.json";

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String StringConfig = "{\"enableWiFi\":0,\"ssid\":\"\",\"pass\":\"\",\"dhcpWiFi\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\",\"disableEmerg\":1}";

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
    String StringConfig = "{\"dhcp\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\",\"disablePingCtrl\":0}";
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
  ConfigSettings.disablePingCtrl = (int)doc["disablePingCtrl"];

  configFile.close();
  return true;
}

bool loadConfigGeneral()
{
  const char *path = "/config/configGeneral.json";

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String deviceID = "SLZB-06";
    //getDeviceID(deviceID);
    String StringConfig = "{\"hostname\":\"" + deviceID + "\",\"disableLeds\": false,\"refreshLogs\":1000,\"usbMode\":0,\"disableLedYellow\":0,\"disableLedBlue\":0}";

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

  if ((double)doc["refreshLogs"] < 1000)
  {
    ConfigSettings.refreshLogs = 1000;
  }
  else
  {
    ConfigSettings.refreshLogs = (double)doc["refreshLogs"];
  }
  strlcpy(ConfigSettings.hostname, doc["hostname"] | "", sizeof(ConfigSettings.hostname));
  ConfigSettings.usbMode = (int)doc["usbMode"];
  ConfigSettings.disableLedYellow = (int)doc["disableLedYellow"];
  ConfigSettings.disableLedBlue = (int)doc["disableLedBlue"];
  ConfigSettings.disableLeds = (int)doc["disableLeds"];
  
  configFile.close();
  return true;
}

bool loadConfigSecurity()
{
  const char *path = "/config/configSecurity.json";

  File configFile = LittleFS.open(path, FILE_READ);
  if (!configFile)
  {
    String StringConfig = "{\"disableWeb\":0,\"webAuth\":0,\"webUser\":"",\"webPass\":""}";

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

void setupWifiAP()
{
  ConfigSettings.wifiModeAP = true;
  ConfigSettings.wifiRetries = 0;

  WiFi.mode(WIFI_AP);
  WiFi.disconnect();

  String AP_NameString;
  getDeviceID(AP_NameString);

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i = 0; i < AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(AP_NameChar); //, WIFIPASS);
  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);
  WiFi.setSleep(false);
  ConfigSettings.wifiAPenblTime = millis();
}

bool setupSTAWifi()
{ 
  ConfigSettings.wifiModeAP = false;
  //DEBUG_PRINT(F("ConfigSettings.wifiRetries "));
  //DEBUG_PRINTLN(ConfigSettings.wifiRetries);
  //if (ConfigSettings.wifiRetries > 3) {
  //    DEBUG_PRINTLN(F("WIFI set but no connect after 3 tries. setupWifiAP()"));
  //    setupWifiAP();
      //modeWiFi = "AP";
  //    ConfigSettings.wifiModeAP = true;
  //    DEBUG_PRINTLN(F("AP"));
      //saveEmergencyWifi(1);
      //DEBUG_PRINTLN(F("ESP.restart"));
      //ESP.restart();
  //}

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
  WiFi.persistent(false);
  WiFi.setHostname(ConfigSettings.hostname);
  if ((strlen(ConfigSettings.ssid) != 0) && (strlen(ConfigSettings.password) != 0))
  {
    DEBUG_PRINTLN(F("Ok SSID & PASS"));
    //setupSTAWifi();
    DEBUG_PRINTLN(F("setupSTAWifi"));
  }
  else
  {
    DEBUG_PRINTLN(F("NO SSID & PASS"));
    setupWifiAP();
    DEBUG_PRINTLN(F("setupWifiAP"));
    //ConfigSettings.wifiModeAP = true;
  }
  //mDNS_start();
}

void setupEthernetAndZigbeeSerial()
{
  ETH.begin(ETH_ADDR_1, ETH_POWER_PIN_1, ETH_MDC_PIN_1, ETH_MDIO_PIN_1, ETH_TYPE_1, ETH_CLK_MODE_1);
  String boardName = "SLZB-06";
  boardName.toCharArray(ConfigSettings.boardName, sizeof(ConfigSettings.boardName));
  DEBUG_PRINTLN(boardName);
  DEBUG_PRINT(F("Zigbee serial setup @ "));
  DEBUG_PRINTLN(ConfigSettings.serialSpeed);
  Serial2.begin(ConfigSettings.serialSpeed, SERIAL_8N1, CC2652P_RXD, CC2652P_TXD);
}

void mDNS_start()
{
  if (!MDNS.begin(ConfigSettings.hostname))
  {
    DEBUG_PRINTLN(F("Error setting up MDNS responder!"));
    // while (1)
    // {
    //   delay(1000);
    // }
  }
  else
  {
    DEBUG_PRINTLN(F("mDNS responder started"));
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("slzb-06", "tcp", ConfigSettings.socketPort);
    MDNS.addServiceTxt("slzb-06", "tcp", "version", "1.0");
    MDNS.addServiceTxt("slzb-06", "tcp", "radio_type", "znp");
    MDNS.addServiceTxt("slzb-06", "tcp", "baud_rate", String(ConfigSettings.serialSpeed));
    MDNS.addServiceTxt("slzb-06", "tcp", "data_flow_control", "software");
  }
}

IRAM_ATTR bool debounce() {
    volatile static unsigned long lastFire = 0;
    if (millis() - lastFire < DEBOUNCE_TIME) {  // Debounce
        return 0;
    }
    lastFire = millis();
    return 1;
}

IRAM_ATTR void btnInterrupt() {
    if (debounce()) btnFlag = true;
}

void setLedsDisable(bool mode, bool setup){
  if(!setup){
    const char *path = "/config/configGeneral.json";
    DynamicJsonDocument doc(300);
    File configFile = LittleFS.open(path, FILE_READ);
    deserializeJson(doc, configFile);
    configFile.close();
    doc["disableLeds"] = mode;
    doc["disableLedYellow"] = mode;
    doc["disableLedBlue"] = mode;
    configFile = LittleFS.open(path, FILE_WRITE);
    serializeJson(doc, configFile);
    configFile.close();
    ConfigSettings.disableLeds = mode;
    ConfigSettings.disableLedYellow = mode;
    ConfigSettings.disableLedBlue = mode;
  }
  if(mode){
    digitalWrite(LED_BLUE, !mode);
    digitalWrite(LED_YELLOW, !mode);
  }else{
    if(!ConfigSettings.disableLedYellow){
      digitalWrite(LED_YELLOW, !mode);
    }else{
      digitalWrite(LED_YELLOW, 0);
    }
    if(ConfigSettings.usbMode && !ConfigSettings.disableLedBlue){
      digitalWrite(LED_BLUE, !mode);
    }else{
      digitalWrite(LED_BLUE, 0);
    }
  }
}

void handlelongBtn() {
    if (!digitalRead(BTN)) {//long press
        printLogMsg("Long press");
        tmrBtnLongPress.stop();
        btnFlag = false;
        setLedsDisable(!ConfigSettings.disableLeds, false);
    } else { //stop long press
        tmrBtnLongPress.stop();
        btnFlag = false;
        printLogMsg("Stop long press");
    }
}

void setUsbMode(bool mode){
  const char *path = "/config/configGeneral.json";
  DynamicJsonDocument doc(300);
  File configFile = LittleFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();
  doc["usbMode"] = mode;
  configFile = LittleFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
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
    ConfigSettings.restarts++;
    DEBUG_PRINT(F("Restarts count "));
    DEBUG_PRINTLN(ConfigSettings.restarts);
    saveRestartCount(ConfigSettings.restarts);
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

  if ((!loadConfigWifi()) || (!loadConfigEther()) || (!loadConfigGeneral()) || (!loadConfigSecurity()))
  {
    DEBUG_PRINTLN(F("Error load config files"));
    ESP.restart();
  }
  else
  {
    configOK = true;
    DEBUG_PRINTLN(F("Config files load OK"));
  }

  pinMode(CC2652P_RST, OUTPUT);
  pinMode(CC2652P_FLSH, OUTPUT);
  digitalWrite(CC2652P_RST, 1);
  digitalWrite(CC2652P_FLSH, 1);
  

  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BTN, INPUT);
  pinMode(MODE_SWITCH, OUTPUT);
  //digitalWrite(LED_YELLOW, !ConfigSettings.disableLedYellow);
  attachInterrupt(digitalPinToInterrupt(BTN), btnInterrupt, FALLING);
  // if (ConfigSettings.disableLedBlue)
  // {
  //   digitalWrite(LED_BLUE, !ConfigSettings.disableLedBlue);
  // }
  // else
  // {
  //   digitalWrite(LED_BLUE, ConfigSettings.usbMode);
  // }
  digitalWrite(MODE_SWITCH, ConfigSettings.usbMode); 
  setLedsDisable(ConfigSettings.disableLeds, true);
  
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

  DEBUG_PRINTLN(millis());

  saveRestartCount(0);

  if (ConfigSettings.restarts > 5)
  { 
    DEBUG_PRINTLN(F("RESET ALL SETTINGS!"));
    resetSettings();
  }
  printLogMsg("Setup done");
}

WiFiClient client[10];

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
    logPush(' ');
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
  logPush(' ');
  logPush('<');
  logPush('-');
  for (int i = 0; i < bytes_read; i++)
  {
    //if (serial_buf[i] == 0x01)
    //{
    //}
    sprintf(output_sprintf, "%02x", serial_buf[i]);
    logPush(' ');
    logPush(output_sprintf[0]);
    logPush(output_sprintf[1]);
    //if (serial_buf[i] == 0x03)
   // {
      
    //}
  }
  logPush('\n');
}

void system_loop()
{
//  if ((millis() > ConfigSettings.wifiAPenblTime + 7*60*1000) && ConfigSettings.wifiModeAP == true && (strlen(ConfigSettings.ssid) != 0) && (strlen(ConfigSettings.password) != 0))
//  {
//    setupSTAWifi();
//  }
}

void loop(void)
{
  uint16_t net_bytes_read = 0;
  uint8_t net_buf[BUFFER_SIZE];

  uint16_t serial_bytes_read = 0;
  uint8_t serial_buf[BUFFER_SIZE];

  if(btnFlag){
    if(!digitalRead(BTN)){//pressed
      if (tmrBtnLongPress.state() == STOPPED){
      tmrBtnLongPress.start();
    }
    }else{
    if (tmrBtnLongPress.state() == RUNNING){
      btnFlag = false;
      tmrBtnLongPress.stop();
      setUsbMode(!ConfigSettings.usbMode);
      printLogMsg("Change usb mode to: " + String(!ConfigSettings.usbMode));
      ESP.restart();
    }
  }
}

tmrBtnLongPress.update();

  system_loop();

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

  if (ConfigSettings.enableWiFi == 0)
  {
    if (ConfigSettings.connectedEther == 0 && ConfigSettings.disconnectEthTime != 0 && ConfigSettings.emergencyWifi != 1 && ConfigSettings.disableEmerg == 0)
    {
      if ((millis() - ConfigSettings.disconnectEthTime) >= (ETH_ERROR_TIME * 1000))
      {
        DEBUG_PRINTLN(F("NO ETH and not enabled WIFI. saveEmergencyWifi(1)"));
        saveEmergencyWifi(1);
        DEBUG_PRINTLN(F("ESP.restart"));
        ESP.restart();
      }
    }
  }
  

  if (server.hasClient())
  {
    for (byte i = 0; i < MAX_SOCKET_CLIENTS; i++)
    {
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

  if (WiFi.getMode() == 2)
  {
    dnsServer.processNextRequest();
  }
}
