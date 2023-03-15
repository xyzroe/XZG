#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
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
#include <esp_wifi.h>
#include <ETH.h>
#include "ESPmDNS.h"
#include <DNSServer.h>

#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif

#define BUFFER_SIZE 256

ConfigSettingsStruct ConfigSettings;
InfosStruct Infos;

volatile bool btnFlag = false;
bool updWeb = false;

const char* coordMode = "coordMode"; //coordMode node name
const char* prevCoordMode = "prevCoordMode"; //prevCoordMode node name
const char* configFileSystem = "/config/system.json";
const char* configFileWifi = "/config/configWifi.json";
const char* configFileEther = "/config/configEther.json";
const char* configFileGeneral = "/config/configGeneral.json";
const char* configFileSecurity = "/config/configSecurity.json";
const char* configFileSerial = "/config/configSerial.json";
const char* deviceModel = "SLZB-06";

void mDNS_start();
void connectWifi();
void handlelongBtn();
void handletmrNetworkOverseer();
void setupCoordinatorMode();
void startAP(const bool start);
IPAddress parse_ip_address(const char *str);

Ticker tmrBtnLongPress(handlelongBtn, 1000, 0, MILLIS);
Ticker tmrNetworkOverseer(handletmrNetworkOverseer, overseerInterval, 0, MILLIS);

IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
WiFiServer server(TCP_LISTEN_PORT, MAX_SOCKET_CLIENTS);
//MDNSResponder MDNS;

void initLan(){
  if ( ETH.begin(ETH_ADDR_1, ETH_POWER_PIN_1, ETH_MDC_PIN_1, ETH_MDIO_PIN_1, ETH_TYPE_1, ETH_CLK_MODE_1)){
      DEBUG_PRINTLN(F("LAN start ok"));
      if (!ConfigSettings.dhcp){
        DEBUG_PRINTLN(F("ETH STATIC"));
        ETH.config(parse_ip_address(ConfigSettings.ipAddress), parse_ip_address(ConfigSettings.ipGW), parse_ip_address(ConfigSettings.ipMask));
        //ConfigSettings.disconnectEthTime = millis();
      }else{
        DEBUG_PRINTLN(F("ETH DHCP"));
      }
    }else{
      DEBUG_PRINTLN(F("LAN start err"));
      //esp_eth_stop();
    }
}

void startSoketServer(){
  server.begin(ConfigSettings.socketPort);
  server.setNoDelay(true);
}

void startServers(bool usb = false){
  initWebServer();
  if(!usb) startSoketServer();
  startAP(false);
  mDNS_start();
}

void handletmrNetworkOverseer(){
  switch (ConfigSettings.coordinator_mode){
  case COORDINATOR_MODE_WIFI:
    DEBUG_PRINTLN(F("WiFi.status()"));
    DEBUG_PRINTLN(WiFi.status());
    if(WiFi.isConnected()){
      DEBUG_PRINTLN(F("WIFI CONNECTED"));
      DEBUG_PRINTLN(F(" "));
      DEBUG_PRINTLN(WiFi.localIP());
      DEBUG_PRINTLN(WiFi.subnetMask());
      DEBUG_PRINTLN(WiFi.gatewayIP());
      startServers();
      tmrNetworkOverseer.stop();
    }else{
      if (tmrNetworkOverseer.counter() > overseerMaxRetry){
        DEBUG_PRINTLN(F("WIFI counter overflow"));
        startAP(true);
        connectWifi();
      }
    }
  break;
  case COORDINATOR_MODE_LAN:
    if (ConfigSettings.connectedEther){
      DEBUG_PRINTLN(F("LAN CONNECTED"));
      startServers();
      tmrNetworkOverseer.stop();
    }else{
      if (tmrNetworkOverseer.counter() > overseerMaxRetry){
        DEBUG_PRINTLN(F("LAN counter overflow"));
        startAP(true);
      }
    }
  break;
  case COORDINATOR_MODE_USB:
    if (tmrNetworkOverseer.counter() > 10){//10 seconds for wifi connect
      if(WiFi.isConnected()){
        tmrNetworkOverseer.stop();
        startServers(true);
      }else{
        initLan();
        if(tmrNetworkOverseer.counter() > 13){//3sec for lan
          if(ConfigSettings.connectedEther){
            tmrNetworkOverseer.stop();
            startServers(true);
          }else{//no network interfaces
            tmrNetworkOverseer.stop();//stop timer
            startAP(true);
          }
        }
      }
    }
  break;
  
  default:
    break;
  }
}

void WiFiEvent(WiFiEvent_t event){ 
  DEBUG_PRINT(F("WiFiEvent "));
  DEBUG_PRINTLN(event);
  switch (event){
  case 18://SYSTEM_EVENT_ETH_START:
    DEBUG_PRINTLN(F("ETH Started"));
    //ConfigSettings.disconnectEthTime = millis();
    ETH.setHostname(ConfigSettings.hostname);
    break;
  case 20://SYSTEM_EVENT_ETH_CONNECTED:
    DEBUG_PRINTLN(F("ETH Connected"));
    break;
  case 22://SYSTEM_EVENT_ETH_GOT_IP:
    DEBUG_PRINTLN(F("ETH MAC: "));
    DEBUG_PRINT(ETH.macAddress());
    DEBUG_PRINT(F(", IPv4: "));
    DEBUG_PRINT(ETH.localIP());
    if (ETH.fullDuplex()){
      DEBUG_PRINT(F(", FULL_DUPLEX"));
    }
    DEBUG_PRINT(F(", "));
    DEBUG_PRINT(ETH.linkSpeed());
    DEBUG_PRINTLN(F("Mbps"));
      ConfigSettings.connectedEther = true;
      //ConfigSettings.disconnectEthTime = 0;
      //mDNS_start();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    DEBUG_PRINTLN(F("SYSTEM_EVENT_STA_GOT_IP"));
  case 21://SYSTEM_EVENT_ETH_DISCONNECTED:
    DEBUG_PRINTLN(F("ETH Disconnected"));
    ConfigSettings.connectedEther = false;
    //ConfigSettings.disconnectEthTime = millis();
    if(tmrNetworkOverseer.state() == STOPPED && ConfigSettings.coordinator_mode == COORDINATOR_MODE_LAN){
      tmrNetworkOverseer.start();
    }
    break;
  case SYSTEM_EVENT_ETH_STOP:
    DEBUG_PRINTLN(F("ETH Stopped"));
    ConfigSettings.connectedEther = false;
    //ConfigSettings.disconnectEthTime = millis();
    if(tmrNetworkOverseer.state() == STOPPED){
      tmrNetworkOverseer.start();
    }
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    DEBUG_PRINTLN(F("WIFI STA DISCONNECTED"));
    if(tmrNetworkOverseer.state() == STOPPED){
      tmrNetworkOverseer.start();
    }
    break;
  default:
    break;
  }
}

IPAddress parse_ip_address(const char *str){
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

bool loadSystemVar(){//todo remove
  File configFile = LittleFS.open(configFileSystem, FILE_READ);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open. try to write defaults"));

    float CPUtemp = getCPUtemp(true);
    int correct = CPUtemp - 30;
    String tempOffset = String(correct);

    String StringConfig = "{\"emergencyWifi\":0,\"tempOffset\":" + tempOffset + "}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LittleFS.open(configFileSystem, FILE_WRITE);
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

  ConfigSettings.tempOffset = (int)doc["tempOffset"];
  if (!ConfigSettings.tempOffset)
  {
    DEBUG_PRINTLN(F("no tempOffset in system.json"));
    configFile.close();

    float CPUtemp = getCPUtemp(true);
    int correct = CPUtemp - 30;
    String tempOffset = String(correct);
    doc["tempOffset"] = int(tempOffset.toInt());

    configFile = LittleFS.open(configFileSystem, FILE_WRITE);
    serializeJson(doc, configFile);
    configFile.close();
    DEBUG_PRINTLN(F("saved tempOffset in system.json"));
    ConfigSettings.tempOffset = int(tempOffset.toInt());
  }
  //ConfigSettings.restarts = (int)doc["restarts"];
  configFile.close();
  return true;
}

bool loadConfigWifi(){
  File configFile = LittleFS.open(configFileWifi, FILE_READ);
  const char* enableWiFi = "enableWiFi";
  const char* ssid = "ssid";
  const char* pass = "pass";
  const char* dhcpWiFi = "dhcpWiFi";
  const char* ip = "ip";
  const char* mask = "mask";
  const char* gw = "gw";
  if (!configFile){
    //String StringConfig = "{\"enableWiFi\":0,\"ssid\":\"\",\"pass\":\"\",\"dhcpWiFi\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\",\"disableEmerg\":1}";
    DynamicJsonDocument doc(1024);
    doc[enableWiFi] = 0;
    doc[ssid] = "";
    doc[pass] = "";
    doc[dhcpWiFi] = 1;
    doc[ip] = "";
    doc[mask] = "";
    doc[gw] = "";
    writeDefultConfig(configFileWifi, doc);
  }

  configFile = LittleFS.open(configFileWifi, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(configFileWifi);
    return false;
  }

  ConfigSettings.dhcpWiFi = (int)doc[dhcpWiFi];
  strlcpy(ConfigSettings.ssid, doc[ssid] | "", sizeof(ConfigSettings.ssid));
  strlcpy(ConfigSettings.password, doc[pass] | "", sizeof(ConfigSettings.password));
  strlcpy(ConfigSettings.ipAddressWiFi, doc[ip] | "", sizeof(ConfigSettings.ipAddressWiFi));
  strlcpy(ConfigSettings.ipMaskWiFi, doc[mask] | "", sizeof(ConfigSettings.ipMaskWiFi));
  strlcpy(ConfigSettings.ipGWWiFi, doc[gw] | "", sizeof(ConfigSettings.ipGWWiFi));
  //ConfigSettings.enableWiFi = (int)doc["enableWiFi"];
  //ConfigSettings.disableEmerg = (int)doc["disableEmerg"];

  configFile.close();
  return true;
}

bool loadConfigEther(){
  const char* dhcp = "dhcp";
  const char* ip = "ip";
  const char* mask = "mask";
  const char* gw = "gw";
  File configFile = LittleFS.open(configFileEther, FILE_READ);
  if (!configFile){
    DynamicJsonDocument doc(1024);
    doc[dhcp] = 1;
    doc[ip] = "";
    doc[mask] = "";
    doc[gw] = "";
    //doc["disablePingCtrl"] = 0;
    //String StringConfig = "{\"dhcp\":1,\"ip\":\"\",\"mask\":\"\",\"gw\":\"\",\"disablePingCtrl\":0}";
    writeDefultConfig(configFileEther, doc);
  }

  configFile = LittleFS.open(configFileEther, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(configFileEther);
    return false;
  }

  ConfigSettings.dhcp = (int)doc[dhcp];
  strlcpy(ConfigSettings.ipAddress, doc[ip] | "", sizeof(ConfigSettings.ipAddress));
  strlcpy(ConfigSettings.ipMask, doc[mask] | "", sizeof(ConfigSettings.ipMask));
  strlcpy(ConfigSettings.ipGW, doc[gw] | "", sizeof(ConfigSettings.ipGW));
  //ConfigSettings.disablePingCtrl = (int)doc["disablePingCtrl"];

  configFile.close();
  return true;
}

bool loadConfigGeneral(){
  const char* hostname = "hostname";
  const char* disableLeds = "disableLeds";
  const char* refreshLogs = "refreshLogs";
  const char* disableLedYellow = "disableLedYellow";
  const char* disableLedBlue = "disableLedBlue";
  const char* prevCoordMode = "prevCoordMode";
  const char* keepWeb = "keepWeb";
  File configFile = LittleFS.open(configFileGeneral, FILE_READ);
  DEBUG_PRINTLN(configFile.readString());
  if (!configFile){
    //String deviceID = deviceModel;
    //getDeviceID(deviceID);
    DEBUG_PRINTLN("RESET ConfigGeneral");
    //String StringConfig = "{\"hostname\":\"" + deviceID + "\",\"disableLeds\": false,\"refreshLogs\":1000,\"usbMode\":0,\"disableLedYellow\":0,\"disableLedBlue\":0,\""+ coordMode +"\":0}\""+ prevCoordMode +"\":0, \"keepWeb\": 0}";
    DynamicJsonDocument doc(1024);
    doc[hostname] = deviceModel;
    doc[disableLeds] = 0;
    doc[refreshLogs] = 1000;
    doc[disableLedYellow] = 0;
    doc[disableLedBlue] = 0;
    doc[coordMode] = 0;
    doc[prevCoordMode] = 0;
    doc[keepWeb] = 0;
    writeDefultConfig(configFileGeneral, doc);
  }

  configFile = LittleFS.open(configFileGeneral, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(configFileGeneral);
    return false;
  }

  if ((double)doc[refreshLogs] < 1000)
  {
    ConfigSettings.refreshLogs = 1000;
  }
  else
  {
    ConfigSettings.refreshLogs = (int)doc[refreshLogs];
  }
  DEBUG_PRINTLN(F("[loadConfigGeneral] 'doc[coordMode]' res is:"));
  DEBUG_PRINTLN(String((uint8_t)doc[coordMode]));
  strlcpy(ConfigSettings.hostname, doc[hostname] | "", sizeof(ConfigSettings.hostname));
  ConfigSettings.coordinator_mode = static_cast<COORDINATOR_MODE_t>((uint8_t)doc[coordMode]);
  ConfigSettings.prevCoordinator_mode = static_cast<COORDINATOR_MODE_t>((uint8_t)doc[prevCoordMode]);
  DEBUG_PRINTLN(F("[loadConfigGeneral] 'static_cast' res is:"));
  DEBUG_PRINTLN(String(ConfigSettings.coordinator_mode));
  ConfigSettings.disableLedYellow = (uint8_t)doc[disableLedYellow];
  DEBUG_PRINTLN(F("[loadConfigGeneral] disableLedYellow"));
  ConfigSettings.disableLedBlue = (uint8_t)doc[disableLedBlue];
  DEBUG_PRINTLN(F("[loadConfigGeneral] disableLedBlue"));
  ConfigSettings.disableLeds = (uint8_t)doc[disableLeds];
  DEBUG_PRINTLN(F("[loadConfigGeneral] disableLeds"));
  ConfigSettings.keepWeb = (uint8_t)doc[keepWeb];
  configFile.close();
  DEBUG_PRINTLN(F("[loadConfigGeneral] config load done"));
  return true;
}

bool loadConfigSecurity(){
  const char* disableWeb = "disableWeb";
  const char* webAuth = "webAuth";
  const char* webUser = "webUser";
  const char* webPass = "webPass";
  const char* fwEnabled = "fwEnabled";
  const char* fwIp = "fwIp";
  File configFile = LittleFS.open(configFileSecurity, FILE_READ);
  if (!configFile){
    //String StringConfig = "{\"disableWeb\":0,\"webAuth\":0,\"webUser\":"",\"webPass\":""}";
    DynamicJsonDocument doc(1024);
    doc[disableWeb] = 0;
    doc[webAuth] = 0;
    doc[webUser] = "admin";
    doc[webPass] = "";
    doc[fwEnabled] = 0;
    doc[fwIp] = "";
    writeDefultConfig(configFileSecurity, doc);
  }

  configFile = LittleFS.open(configFileSecurity, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(configFileSecurity);
    return false;
  }

  ConfigSettings.disableWeb = (uint8_t)doc[disableWeb];
  ConfigSettings.webAuth = (uint8_t)doc[webAuth];
  strlcpy(ConfigSettings.webUser, doc[webUser] | "", sizeof(ConfigSettings.webUser));
  strlcpy(ConfigSettings.webPass, doc[webPass] | "", sizeof(ConfigSettings.webPass));
  ConfigSettings.fwEnabled = (uint8_t)doc[fwEnabled];
  ConfigSettings.fwIp = parse_ip_address(doc[fwIp] | "0.0.0.0");

  configFile.close();
  return true;
}

bool loadConfigSerial(){
  const char* baud = "baud";
  const char* port = "port";
  File configFile = LittleFS.open(configFileSerial, FILE_READ);
  if (!configFile){
    //String StringConfig = "{\"baud\":115200,\"port\":6638}";
    DynamicJsonDocument doc(1024);
    doc[baud] = 115200;
    doc[port] = 6638;
    writeDefultConfig(configFileSerial, doc);
  }

  configFile = LittleFS.open(configFileSerial, FILE_READ);
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configFile);

  if (error)
  {
    DEBUG_PRINTLN(F("deserializeJson() failed: "));
    DEBUG_PRINTLN(error.f_str());

    configFile.close();
    LittleFS.remove(configFileSerial);
    return false;
  }

  ConfigSettings.serialSpeed = (int)doc[baud];
  ConfigSettings.socketPort = (int)doc[port];
  if (ConfigSettings.socketPort == 0)
  {
    ConfigSettings.socketPort = TCP_LISTEN_PORT;
  }
  configFile.close();
  return true;
}

void startAP(const bool start){
  if (ConfigSettings.apStarted){
    if (!start){
      if(ConfigSettings.coordinator_mode != COORDINATOR_MODE_WIFI){
        WiFi.softAPdisconnect(true);//off wifi
      }else{
        WiFi.mode(WIFI_STA);
      }
      dnsServer.stop();
      ConfigSettings.apStarted = false;
    }
  }else{
    if(!start) return;
    WiFi.mode(WIFI_AP_STA);//WIFI_AP_STA for possible wifi scan in wifi mode
    WiFi.disconnect();
    // String AP_NameString;
    // getDeviceID(AP_NameString);

    // char AP_NameChar[AP_NameString.length() + 1];
    // memset(AP_NameChar, 0, AP_NameString.length() + 1);

    // for (int i = 0; i < AP_NameString.length(); i++){
    //   AP_NameChar[i] = AP_NameString.charAt(i);
    // }
      
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    char apSsid[18];
    getDeviceID(apSsid);
    WiFi.softAP(apSsid); //, WIFIPASS);
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", apIP);
    WiFi.setSleep(false);
    //ConfigSettings.wifiAPenblTime = millis();
    startServers();
    ConfigSettings.apStarted = true;
  }
}

void connectWifi(){
  static uint8_t timeout = 0;
  if (WiFi.status() == WL_IDLE_STATUS && timeout < 20){//connection in progress
    DEBUG_PRINTLN(F("[connectWifi] WL_IDLE_STATUS"));
    timeout++;
    return; 
  }else{
    timeout = 0;
    DEBUG_PRINTLN(F("[connectWifi] timeout"));
  }
  WiFi.persistent(false);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B);
  if ((strlen(ConfigSettings.ssid) >= 2) && (strlen(ConfigSettings.password) >= 8)){
    DEBUG_PRINTLN(F("[connectWifi] Ok SSID & PASS"));
    if (ConfigSettings.apStarted){
      // DEBUG_PRINTLN(F("[connectWifi] WiFi.mode(WIFI_AP_STA)"));
      // WiFi.mode(WIFI_AP_STA);
    }else{
      DEBUG_PRINTLN(F("[connectWifi] WiFi.mode(WIFI_STA)"));
      WiFi.mode(WIFI_STA);
    }
    delay(100);

    WiFi.begin(ConfigSettings.ssid, ConfigSettings.password);
    WiFi.setSleep(false);
    DEBUG_PRINTLN(F("[connectWifi] WiFi.begin"));

    if (!ConfigSettings.dhcpWiFi){
      IPAddress ip_address = parse_ip_address(ConfigSettings.ipAddressWiFi);
      IPAddress gateway_address = parse_ip_address(ConfigSettings.ipGWWiFi);
      IPAddress netmask = parse_ip_address(ConfigSettings.ipMaskWiFi);
      WiFi.config(ip_address, gateway_address, netmask);
      DEBUG_PRINTLN(F("[connectWifi] WiFi.config"));
    }else{
      DEBUG_PRINTLN(F("[connectWifi] Try DHCP"));
    }
  }else{
    if(!(ConfigSettings.coordinator_mode == COORDINATOR_MODE_USB && ConfigSettings.keepWeb)){//dont start ap in keepWeb
      DEBUG_PRINTLN(F("[connectWifi] NO SSID & PASS"));
      startAP(true);
      DEBUG_PRINTLN(F("[connectWifi] setupWifiAP"));
    }
  }
}

void mDNS_start()
{
  const char* host = "_slzb-06";// PR#84111
  const char* http = "_http";
  const char* tcp = "_tcp";
  if (!MDNS.begin(ConfigSettings.hostname))
  {
    printLogMsg("Error setting up MDNS responder!");
  }else{
    printLogMsg("mDNS responder started");
    MDNS.addService(http, tcp, 80);//web
    //--zeroconf zha--
    MDNS.addService(host, tcp, ConfigSettings.socketPort);
    MDNS.addServiceTxt(host, tcp, "version", "1.0");
    MDNS.addServiceTxt(host, tcp, "radio_type", "znp");
    MDNS.addServiceTxt(host, tcp, "baud_rate", String(ConfigSettings.serialSpeed));
    MDNS.addServiceTxt(host, tcp, "data_flow_control", "software");
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
  DEBUG_PRINTLN(F("[setLedsDisable] start"));
  if(!setup){
    const char *path = configFileGeneral;
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
    if(ConfigSettings.coordinator_mode == COORDINATOR_MODE_USB && !ConfigSettings.disableLedBlue){
      digitalWrite(LED_BLUE, !mode);
    }else{
      digitalWrite(LED_BLUE, 0);
    }
  }
  DEBUG_PRINTLN(F("[setLedsDisable] done"));
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

void toggleUsbMode(){
  DEBUG_PRINTLN(F("prevCoordMode"));
  DEBUG_PRINTLN(prevCoordMode);
  DEBUG_PRINTLN(F("coordMode"));
  DEBUG_PRINTLN(coordMode);
  if(ConfigSettings.coordinator_mode != COORDINATOR_MODE_USB){
    ConfigSettings.prevCoordinator_mode = ConfigSettings.coordinator_mode; //remember current state
    ConfigSettings.coordinator_mode = COORDINATOR_MODE_USB; //toggle
    DEBUG_PRINTLN(F("Change usb mode to USB"));
  }else{
    ConfigSettings.coordinator_mode = ConfigSettings.prevCoordinator_mode;
    DEBUG_PRINTLN(F("Change usb mode to:"));
    DEBUG_PRINTLN(String(ConfigSettings.coordinator_mode));
  }
  const char *path = configFileGeneral;
  DynamicJsonDocument doc(300);
  File configFile = LittleFS.open(path, FILE_READ);
  deserializeJson(doc, configFile);
  configFile.close();
  doc[prevCoordMode] = ConfigSettings.prevCoordinator_mode;
  doc[coordMode] = ConfigSettings.coordinator_mode;
  configFile = LittleFS.open(path, FILE_WRITE);
  serializeJson(doc, configFile);
  configFile.close();
  digitalWrite(LED_BLUE, ConfigSettings.coordinator_mode == COORDINATOR_MODE_USB ? 1 : 0);
  ESP.restart();
}

void setupCoordinatorMode(){
  if (ConfigSettings.coordinator_mode > 2 || ConfigSettings.coordinator_mode < 0){
    DEBUG_PRINTLN(F("WRONG MODE DETECTED, set to LAN"));
    ConfigSettings.coordinator_mode = COORDINATOR_MODE_LAN;
  }
  DEBUG_PRINTLN(F("setupCoordinatorMode"));
  DEBUG_PRINTLN(F("Mode is:"));
  DEBUG_PRINTLN(ConfigSettings.coordinator_mode);
  DEBUG_PRINTLN(F("--------------"));
  if(ConfigSettings.coordinator_mode != COORDINATOR_MODE_USB || ConfigSettings.keepWeb){//start network overseer
    if(tmrNetworkOverseer.state() == STOPPED){
      tmrNetworkOverseer.start();
    }
    WiFi.onEvent(WiFiEvent);
  }
  switch (ConfigSettings.coordinator_mode){
    case COORDINATOR_MODE_USB:
      DEBUG_PRINTLN(F("Coordinator USB mode"));
      digitalWrite(MODE_SWITCH, 1);
    break;

    case COORDINATOR_MODE_WIFI:
      DEBUG_PRINTLN(F("Coordinator WIFI mode"));
      connectWifi();
    break;

    case COORDINATOR_MODE_LAN:
      DEBUG_PRINTLN(F("Coordinator LAN mode"));
      initLan();
    break;
  
  default:
    break;
  }
  if(!ConfigSettings.disableWeb && (ConfigSettings.coordinator_mode != COORDINATOR_MODE_USB || ConfigSettings.keepWeb)) updWeb = true;//handle web server
  if(ConfigSettings.coordinator_mode == COORDINATOR_MODE_USB && ConfigSettings.keepWeb) connectWifi();//try 2 connect wifi
}

// void cmd2zigbee(const HardwareSerial serial, byte cmd[], const byte size){
//   byte checksum;
//   for (byte i = 1; i < size - 1; i++){
//     checksum ^= cmd[i];
//   }
//   cmd[size] = checksum;
//   serial.write(cmd, size);
// }

void clearS2Buffer(){
  while (Serial2.available()){//clear buffer
    Serial2.read();
  }
}

void setup(){
  Serial.begin(115200);//todo ifdef DEBUG
  ConfigSettings.apStarted = false;
  ConfigSettings.serialSpeed = 115200;
  DEBUG_PRINTLN(F("Start"));
  pinMode(CC2652P_RST, OUTPUT);
  pinMode(CC2652P_FLSH, OUTPUT);
  digitalWrite(CC2652P_RST, 1);
  digitalWrite(CC2652P_FLSH, 1);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BTN, INPUT);
  pinMode(MODE_SWITCH, OUTPUT);
  digitalWrite(MODE_SWITCH, 0);//enable zigbee serial
  digitalWrite(LED_YELLOW, 1);
  digitalWrite(LED_BLUE, 1);

  //hard reset
  if(!digitalRead(BTN)){
    DEBUG_PRINTLN(F("[hard reset] Entering hard reset mode"));
    uint8_t counter = 0;
    while (!digitalRead(BTN)){
      if (counter >= 10){
        resetSettings();
      }else{
        counter++;
        DEBUG_PRINTLN(counter);
        delay(200);
      }
    }
    DEBUG_PRINTLN(F("[hard reset] Btn up, exit"));
  }
  //--------------------

  //zig connection & leds testing 
  const byte cmdLed0 = 0x27;
  const byte cmdLed1 = 0x0A;
  const byte cmdLedIndex = 0x01;//for led 1
  const byte cmdLedStateOff = 0x00;
  const byte cmdLedStateOn = 0x01;
  const byte cmdFrameStart = 0xFE;
  const byte cmdLedLen = 0x02;
  const byte zigLed1Off[] = {cmdFrameStart, cmdLedLen, cmdLed0, cmdLed1, cmdLedIndex, cmdLedStateOff, 0x2E}; //resp FE 01 67 0A 00 6C
  const byte zigLed1On[] = {cmdFrameStart, cmdLedLen, cmdLed0, cmdLed1, cmdLedIndex, cmdLedStateOn, 0x2F};
  const byte cmdLedResp[] = {0xFE, 0x01, 0x67, 0x0A, 0x00, 0x6C};
  Serial2.begin(115200, SERIAL_8N1, CC2652P_RXD, CC2652P_TXD); //start zigbee serial
  bool respOk = false;
  for (uint8_t i = 0; i < 12; i++){//wait for zigbee start
    if(respOk) break;
    clearS2Buffer();
    Serial2.write(zigLed1On, sizeof(zigLed1On));
    Serial2.flush();
    delay(400);
    for (uint8_t i = 0; i < 5; i++){
      if (Serial2.read() != 0xFE){//check for packet start
        Serial2.read();//skip
      }else{
        for (uint8_t i = 1; i < 4; i++){
          if(Serial2.read() != cmdLedResp[i]){//check if resp ok
            respOk = false;
            break;
          }else{
            respOk = true;
          }
        }
      }
    }
    digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));//blue led flashing mean wait for zigbee resp
  }
  delay(500);
  if(!respOk){
    digitalWrite(LED_YELLOW, 1);
    digitalWrite(LED_BLUE, 1);
    for (uint8_t i = 0; i < 5; i++){//indicate wrong resp
          digitalWrite(LED_YELLOW, !digitalRead(LED_YELLOW));
          digitalWrite(LED_BLUE, !digitalRead(LED_BLUE));
          delay(1000);
        }
    printLogMsg("[ZBCHK] Wrong answer");
  }else{
    Serial2.write(zigLed1Off, sizeof(zigLed1Off));
    Serial2.flush();
    printLogMsg("[ZBCHK] connection established");
  }
  digitalWrite(LED_YELLOW, 0);
  digitalWrite(LED_BLUE, 0);
  //-----------------

  attachInterrupt(digitalPinToInterrupt(BTN), btnInterrupt, FALLING);

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
    // ConfigSettings.restarts++;
    // DEBUG_PRINT(F("Restarts count "));
    // DEBUG_PRINTLN(ConfigSettings.restarts);
    // saveRestartCount(ConfigSettings.restarts);
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

  if ((!loadConfigWifi()) || (!loadConfigEther()) || (!loadConfigGeneral()) || (!loadConfigSecurity()))
  {
    DEBUG_PRINTLN(F("Error load config files"));
    ESP.restart();
  }
  else
  {
    DEBUG_PRINTLN(F("Config files load OK"));
  }
  DEBUG_PRINTLN("ConfigSettings.disableLeds: ");
  DEBUG_PRINTLN(ConfigSettings.disableLeds);
  setLedsDisable(ConfigSettings.disableLeds, true);
  setupCoordinatorMode();
  ConfigSettings.connectedClients = 0;

  Serial2.updateBaudRate(ConfigSettings.serialSpeed);//set actual speed
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
    unsigned long timeLog = millis();
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
  unsigned long timeLog = millis();
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

void loop(void){
  if(btnFlag){
    if(!digitalRead(BTN)){//pressed
      if (tmrBtnLongPress.state() == STOPPED){
      tmrBtnLongPress.start();
    }
    }else{
      if (tmrBtnLongPress.state() == RUNNING){
        btnFlag = false;
        tmrBtnLongPress.stop();
        toggleUsbMode();
      }
    }
  }

  tmrBtnLongPress.update();
  tmrNetworkOverseer.update();
  if (updWeb){
    webServerHandleClient();
  }else{
    if (ConfigSettings.connectedClients == 0){
      webServerHandleClient();
    }
  }

  if (ConfigSettings.coordinator_mode != COORDINATOR_MODE_USB){
    uint16_t net_bytes_read = 0;
    uint8_t net_buf[BUFFER_SIZE];
    uint16_t serial_bytes_read = 0;
    uint8_t serial_buf[BUFFER_SIZE];

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
        if(ConfigSettings.fwEnabled){
          WiFiClient TempClient2 = server.available();
          if(TempClient2.remoteIP() == ConfigSettings.fwIp){
            printLogMsg(String("[SOCK IP WHITELIST] Accepted connection from IP: ") + TempClient2.remoteIP().toString());
            client[i] = TempClient2;
            continue;
          }else{
            printLogMsg(String("[SOCK IP WHITELIST] Rejected connection from unknown IP: ") + TempClient2.remoteIP().toString());
          }
        }else{
          client[i] = server.available();
          continue;
        }
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
  }

  if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA)
  {
    dnsServer.processNextRequest();
  }
}
