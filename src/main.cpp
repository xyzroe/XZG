#include <WiFi.h>
// #include <WiFiClient.h>
// #include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <LittleFS.h>
#include <Update.h>
#include <Ticker.h>
#include <esp_wifi.h>
#include <ETH.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <CCTools.h>
#include <WireGuard-ESP32.h>
#include <CronAlarms.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "mqtt.h"
#include "zb.h"
#include "version.h"
// #include "hw.h"

#include "esp_system.h"
#include "esp_task_wdt.h"

/*
#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif
*/

// ConfigSettingsStruct ConfigSettings;
// MqttSettingsStruct MqttSettings;
// WgSettingsStruct WgSettings;
// CurrentModesStruct modes;

extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

LEDControl ledControl;

BrdConfigStruct hwConfig;

SystemConfigStruct systemCfg;
NetworkConfigStruct networkCfg;
VpnConfigStruct vpnCfg;
MqttConfigStruct mqttCfg;

SysVarsStruct vars;

zbVerStruct zbVer;

// volatile bool btnFlag = false;
int btnFlag = false;
bool updWeb = false;

void mDNS_start();
void connectWifi();
void handleLongBtn();
void handleTmrNetworkOverseer();
void setupCoordinatorMode();
void startAP(const bool start);
void toggleUsbMode();

Ticker tmrBtnLongPress(handleLongBtn, 1000, 0, MILLIS);
Ticker tmrNetworkOverseer(handleTmrNetworkOverseer, overseerInterval, 0, MILLIS);

IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
WiFiServer server(ZB_TCP_PORT, MAX_SOCKET_CLIENTS);

CCTools CCTool(Serial2);

// MDNSResponder MDNS; don't need?

void initLan()
{

  DEBUG_PRINT(F("Some ETH config found. Try to use "));
  DEBUG_PRINTLN(hwConfig.board);
  if (ETH.begin(hwConfig.eth.addr, hwConfig.eth.pwrPin, hwConfig.eth.mdcPin, hwConfig.eth.mdiPin, hwConfig.eth.phyType, hwConfig.eth.clkMode, hwConfig.eth.pwrAltPin))
  {
    DEBUG_PRINTLN(F("LAN start ok"));
    if (!networkCfg.ethDhcp)
    {
      DEBUG_PRINTLN(F("ETH STATIC"));
      ETH.config(networkCfg.ethIp, networkCfg.ethGate, networkCfg.ethMask, networkCfg.ethDns1, networkCfg.ethDns2);
      // ConfigSettings.disconnectEthTime = millis();
    }
    else
    {
      DEBUG_PRINTLN(F("ETH DHCP"));
    }
  }
  else
  {
    DEBUG_PRINTLN(F("LAN start err"));
    // esp_eth_stop();
  }
}

/*
void initZb()
{

  const char *txPin = "txPin";
  const char *rxPin = "rxPin";
  const char *rstPin = "rstPin";
  const char *bslPin = "bslPin";

  File configFile = LittleFS.open(configFileHw, FILE_READ);

  DynamicJsonDocument config(1024);
  deserializeJson(config, configFile);
  configFile.close();

  ZbConfig hwConfig;
  hwConfig.txPin = config[txPin];
  hwConfig.rxPin = config[rxPin];
  hwConfig.rstPin = config[rstPin];
  hwConfig.bslPin = config[bslPin];

  if (hwConfig.txPin && hwConfig.rxPin && hwConfig.rstPin && hwConfig.bslPin)
  {
    DEBUG_PRINTLN(F("Some ZB config found. Try to use"));

    Serial2.begin(systemCfg.serialSpeed, SERIAL_8N1, hwConfig.rxPin, hwConfig.txPin); // start zigbee serial

    int BSL_MODE = 0;

    if (zbInit(hwConfig.rstPin, hwConfig.bslPin, BSL_MODE))
    {
      DEBUG_PRINTLN(F("Zigbee init - OK"));
    }
    else
    {
      DEBUG_PRINTLN(F("Zigbee init - ERROR"));
    }
  }
  else
  {
    // DynamicJsonDocument doc(512);
    // doc[txPin] = 0;
    // doc[rxPin] = 0;
    // doc[rstPin] = 0;
    // doc[bslPin] = 0;
    // writeDefaultConfig(configFileHw, doc);
    const char *pwrPin = "pwrPin";
    const char *mdcPin = "mdcPin";
    const char *mdiPin = "mdiPin";
    const char *clkMode = "clkMode";
    const char *pwrAltPin = "pwrAltPin";
    eth_clock_mode_t clkModeEth = config[clkMode];

    int clkPin;
    if (clkModeEth == ETH_CLOCK_GPIO0_IN)
      clkPin = 0;
    if (clkModeEth == ETH_CLOCK_GPIO0_OUT)
      clkPin = 0;
    if (clkModeEth == ETH_CLOCK_GPIO16_OUT)
      clkPin = 16;
    if (clkModeEth == ETH_CLOCK_GPIO17_OUT)
      clkPin = 17;

    DEBUG_PRINTLN(F("NO ZB config in memory"));
    ZbConfig *newConfig = findZbConfig(config[pwrPin], config[mdcPin], config[mdiPin], clkPin, config[pwrAltPin]);
    if (newConfig)
    {
      DEBUG_PRINTLN(F("Saving ZB HW config"));

      DynamicJsonDocument config(1024);
      File configFile = LittleFS.open(configFileHw, FILE_READ);
      deserializeJson(config, configFile);
      configFile.close();

      config[txPin] = newConfig->txPin;
      config[rxPin] = newConfig->rxPin;
      config[rstPin] = newConfig->rstPin;
      config[bslPin] = newConfig->bslPin;

      configFile = LittleFS.open(configFileHw, FILE_WRITE);
      serializeJson(config, configFile);
      configFile.close();
      // delay(500);
      DEBUG_PRINTLN(F("Restarting..."));
      esp_restart();
    }
    else
    {
      DEBUG_PRINTLN(F("findZbConfig - NO SUCCESS"));
    }
  }
}
*/

void startSocketServer()
{
  server.begin(systemCfg.socketPort);
  server.setNoDelay(true);
}

void startServers(bool usb = false)
{
  initWebServer();

  startAP(false);
  mDNS_start();
  zbFwCheck();
  if (!vars.apStarted)
  {
    if (!usb)
    {
      startSocketServer();
    }
    if (vpnCfg.wgEnable)
    {
      wgBegin();
    }
    if (mqttCfg.enable)
    {
      mqttConnectSetup();
    }
    xTaskCreate(setClock, "setClock", 2048, NULL, 9, NULL);
  }

  /* //not available now
  if (vpnCfg.hnEnable)
  {
    hnBegin();
  }
  */
}

void handleTmrNetworkOverseer()
{
  switch (systemCfg.workMode)
  {
  case WORK_MODE_NETWORK:
    if (!networkCfg.wifiEnable && !networkCfg.ethEnable)
    {
      DEBUG_PRINTLN(F("Both interfaces disabled. Start AP"));
      startAP(true);
      connectWifi();
    }
    if (networkCfg.wifiEnable)
    {
      DEBUG_PRINT(F("WiFi.status = "));
      DEBUG_PRINTLN(WiFi.status());
      if (WiFi.isConnected())
      {
        DEBUG_PRINTLN(F("WIFI CONNECTED"));
        startServers();
        tmrNetworkOverseer.stop();
      }
      else
      {
        if (tmrNetworkOverseer.counter() > overseerMaxRetry)
        {
          DEBUG_PRINTLN(F("WIFI counter overflow"));
          startAP(true);
          connectWifi();
        }
      }
    }
    // break;
    // case WORK_MODE_NETWORK:
    if (networkCfg.ethEnable)
    {
      if (vars.connectedEther)
      {
        DEBUG_PRINTLN(F("LAN CONNECTED"));
        startServers();
        tmrNetworkOverseer.stop();
      }
      else
      {
        if (tmrNetworkOverseer.counter() > overseerMaxRetry)
        {
          DEBUG_PRINTLN(F("LAN counter overflow"));
          startAP(true);
        }
      }
    }
    break;
  case WORK_MODE_USB:
    if (tmrNetworkOverseer.counter() > 3)
    { // 10 seconds for wifi connect
      if (WiFi.isConnected())
      {
        tmrNetworkOverseer.stop();
        startServers(true);
      }
      else
      {
        initLan();
        if (tmrNetworkOverseer.counter() > 6)
        { // 3sec for lan
          if (vars.connectedEther)
          {
            tmrNetworkOverseer.stop();
            startServers(true);
          }
          else
          {                            // no network interfaces
            tmrNetworkOverseer.stop(); // stop timer
            startAP(true);
          }
        }
      }
    }
    break;

    // default:
    // break;
  }
}

void NetworkEvent(WiFiEvent_t event)
{
  const char *wifiKey = "WiFi";
  const char *ethKey = "ETH";

  switch (event)
  {
  case ARDUINO_EVENT_ETH_START: // 18: // SYSTEM_EVENT_ETH_START:
    LOGI(ethKey, "Started");
    // DEBUG_PRINTLN(F("ETH Started"));
    //  ConfigSettings.disconnectEthTime = millis();
    ETH.setHostname(systemCfg.hostname);
    break;
  case ARDUINO_EVENT_ETH_CONNECTED: // 20: // SYSTEM_EVENT_ETH_CONNECTED:
    // DEBUG_PRINTLN(F("ETH Connected"));
    LOGI(ethKey, "Connected");
    break;
  case ARDUINO_EVENT_ETH_GOT_IP: // 22: // SYSTEM_EVENT_ETH_GOT_IP:

    LOGI(ethKey, "MAC: %s, IP: %s, Mask: %s, Gw: %s, %dMbps",
         ETH.macAddress().c_str(),
         ETH.localIP().toString().c_str(),
         ETH.subnetMask().toString().c_str(),
         ETH.gatewayIP().toString().c_str(),
         ETH.linkSpeed());

    vars.connectedEther = true;
    // ConfigSettings.disconnectEthTime = 0;
    // mDNS_start();
    // setClock();
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED: // 21:  //SYSTEM_EVENT_ETH_DISCONNECTED:
    // DEBUG_PRINTLN(F("ETH Disconnected"));
    LOGI(ethKey, "Disconnected");
    vars.connectedEther = false;
    // ConfigSettings.disconnectEthTime = millis();
    if (tmrNetworkOverseer.state() == STOPPED && systemCfg.workMode == WORK_MODE_NETWORK)
    {
      tmrNetworkOverseer.start();
    }
    break;
  case SYSTEM_EVENT_ETH_STOP: // 27:
  case ARDUINO_EVENT_ETH_STOP:
    LOGI(ethKey, "Stopped");
    // DEBUG_PRINTLN(F("ETH Stopped"));
    vars.connectedEther = false;
    // ConfigSettings.disconnectEthTime = millis();
    if (tmrNetworkOverseer.state() == STOPPED)
    {
      tmrNetworkOverseer.start();
    }
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP: // SYSTEM_EVENT_STA_GOT_IP:

    /*
    DEBUG_PRINT(F("WiFi MAC: "));
    DEBUG_PRINT(WiFi.macAddress());
    DEBUG_PRINT(F(", IPv4: "));
    DEBUG_PRINT(WiFi.localIP().toString());
    DEBUG_PRINT(F(", "));
    DEBUG_PRINT(WiFi.subnetMask().toString());
    DEBUG_PRINT(F(", "));
    DEBUG_PRINTLN(WiFi.gatewayIP().toString());
    */

    LOGI(wifiKey, "MAC: %s, IP: %s, Mask: %s, Gw: %s",
         WiFi.macAddress().c_str(),
         WiFi.localIP().toString().c_str(),
         WiFi.subnetMask().toString().c_str(),
         WiFi.gatewayIP().toString().c_str());

    // setClock();
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: // SYSTEM_EVENT_STA_DISCONNECTED:
    LOGI(wifiKey, "STA DISCONNECTED");
    // DEBUG_PRINTLN(F("WIFI "));
    if (tmrNetworkOverseer.state() == STOPPED)
    {
      tmrNetworkOverseer.start();
    }
    break;
  default:
    break;
  }
}

void startAP(const bool start)
{
  String tag = "sAP";
  LOGI(tag, "begin s=%d, v=%d", start, vars.apStarted);

  if (vars.apStarted)
  {
    if (!start)
    {
      if (!networkCfg.wifiEnable)
      {
        DEBUG_PRINTLN("1");
        WiFi.softAPdisconnect(true); // off wifi
      }
      else
      {
        DEBUG_PRINTLN("2");
        WiFi.mode(WIFI_STA);
      }
      DEBUG_PRINTLN("3");
      dnsServer.stop();
      vars.apStarted = false;
    }
  }
  else
  {
    if (!start)
      return;
    LOGI(tag, "WIFI_AP_STA");
    WiFi.mode(WIFI_AP_STA); // WIFI_AP_STA for possible wifi scan in wifi mode
    WiFi.disconnect();
    // String AP_NameString;
    // getDeviceID(AP_NameString);

    // char AP_NameChar[AP_NameString.length() + 1];
    // memset(AP_NameChar, 0, AP_NameString.length() + 1);

    // for (int i = 0; i < AP_NameString.length(); i++){
    //   AP_NameChar[i] = AP_NameString.charAt(i);
    // }

    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    // char apSsid[MAX_DEV_ID_LONG];
    // getDeviceID(apSsid);
    WiFi.softAP(vars.deviceId); //, WIFIPASS);
    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", apIP);
    WiFi.setSleep(false);
    // ConfigSettings.wifiAPenblTime = millis();
    LOGI(tag, "startServers()");
    startServers();
    vars.apStarted = true;
  }
}

void connectWifi()
{
  static uint8_t timeout = 0;
  if (WiFi.status() == WL_IDLE_STATUS && timeout < 20)
  { // connection in progress
    DEBUG_PRINTLN(F("[connectWifi] WL_IDLE_STATUS"));
    timeout++;
    return;
  }
  else
  {
    timeout = 0;
    DEBUG_PRINTLN(F("[connectWifi] timeout"));
  }
  WiFi.persistent(false);
  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B);
  if ((strlen(networkCfg.wifiSsid) >= 2) && (strlen(networkCfg.wifiPass) >= 8))
  {
    DEBUG_PRINTLN(F("[connectWifi] Ok SSID & PASS"));
    if (vars.apStarted)
    {
      // DEBUG_PRINTLN(F("[connectWifi] WiFi.mode(WIFI_AP_STA)"));
      // WiFi.mode(WIFI_AP_STA);
    }
    else
    {
      DEBUG_PRINTLN(F("[connectWifi] WiFi.mode(WIFI_STA)"));
      WiFi.mode(WIFI_STA);
    }
    delay(100);

    WiFi.begin(networkCfg.wifiSsid, networkCfg.wifiPass);
    WiFi.setSleep(false);
    DEBUG_PRINTLN(F("[connectWifi] WiFi.begin"));

    if (!networkCfg.wifiDhcp)
    {
      WiFi.config(networkCfg.wifiIp, networkCfg.wifiGate, networkCfg.wifiMask, networkCfg.wifiDns1, networkCfg.wifiDns2);
      DEBUG_PRINTLN(F("[connectWifi] WiFi.config"));
    }
    else
    {
      DEBUG_PRINTLN(F("[connectWifi] Try DHCP"));
    }
  }
  else
  {
    if (!(systemCfg.workMode == WORK_MODE_USB && systemCfg.keepWeb))
    { // dont start ap in keepWeb
      DEBUG_PRINT(F("[connectWifi] NO SSID & PASS "));
      if (!vars.connectedEther)
      {
        DEBUG_PRINTLN(F("and problem with LAN"));
        startAP(true);
        DEBUG_PRINTLN(F("So [connectWifi] setupWifiAP"));
      }
      else
      {
        DEBUG_PRINTLN(F("but LAN is OK"));
      }
    }
  }
}

void mDNS_start()
{
  String tag = "mDNS";
  const char *host = "_xzg";
  const char *http = "_http";
  const char *tcp = "_tcp";
  if (!MDNS.begin(systemCfg.hostname))
  {
    String msg = "Error setting up MDNS responder!";
    LOGI(tag, "%s", msg.c_str());
    // printLogMsg(msg);
  }
  else
  {
    String msg = "mDNS responder started";
    LOGI(tag, "%s", msg.c_str());
    // printLogMsg(msg);
    MDNS.addService(http, tcp, 80); // web
    //--zeroconf zha--
    MDNS.addService(host, tcp, systemCfg.socketPort);
    MDNS.addServiceTxt(host, tcp, "version", "1.0");
    MDNS.addServiceTxt(host, tcp, "radio_type", "znp");
    MDNS.addServiceTxt(host, tcp, "baud_rate", String(systemCfg.serialSpeed));
    MDNS.addServiceTxt(host, tcp, "data_flow_control", "software");
    MDNS.addServiceTxt(host, tcp, "board", String(hwConfig.board));
    // msg = "setup finish";
    // LOGI(tag, "%s", msg);
  }
}

IRAM_ATTR bool debounce()
{
  volatile static unsigned long lastFire = 0;
  if (millis() - lastFire < DEBOUNCE_TIME)
  { // Debounce
    return 0;
  }
  lastFire = millis();
  return 1;
}

IRAM_ATTR void btnInterrupt()
{
  if (debounce())
  {
    if (!btnFlag)
    {
      btnFlag = true;
    }
  }
}

void handleLongBtn()
{
  String tag = "lBTN";
  if (!digitalRead(hwConfig.mist.btnPin))
  {
    LOGI(tag, "press +, %d s", btnFlag);

    btnFlag++;
    if (btnFlag >= 3)
    {
      ledControl.modeLED.mode = LED_FLASH_1Hz;
    }
  }
  else
  {
    LOGI(tag, "press -, %d s", btnFlag);

    if (btnFlag >= 3)
    {

      printLogMsg("BTN - 3sec - toggleUsbMode");
      toggleUsbMode();
    }
    else
    {
      printLogMsg("BTN - click - setLedsDisable");

      setLedsDisable(!vars.disableLeds);
      vars.disableLeds = !vars.disableLeds;
    }
    tmrBtnLongPress.stop();
    btnFlag = false;
  }
  if (btnFlag >= 5)
  {
    ledControl.modeLED.mode = LED_FLASH_3Hz;
    printLogMsg("BTN - 5sec - zigbeeEnableBSL");
    zigbeeEnableBSL();
    tmrBtnLongPress.stop();
    btnFlag = false;
  }
}

void toggleUsbMode()
{

  String tag = "tUSB";
  LOGI(tag, "start");
  delay(250);
  if (systemCfg.workMode != WORK_MODE_USB)
  {
    systemCfg.workMode = WORK_MODE_USB;
  }
  else
  {
    systemCfg.workMode = WORK_MODE_NETWORK;
  }
  saveSystemConfig(systemCfg);
  LOGI(tag, "Change mode to %s", String(systemCfg.workMode));

  if (vars.hwLedUsbIs)
  {
    // ledWrite(MODE_LED, vars.workMode == WORK_MODE_USB ? ON : OFF);
    ledControl.modeLED.mode = LED_ON;
  }
  ESP.restart();
}

void setupCoordinatorMode()
{
  if (systemCfg.workMode > 2 || systemCfg.workMode < 0)
  {
    DEBUG_PRINTLN(F("WRONG MODE DETECTED, set to LAN"));
    systemCfg.workMode = WORK_MODE_NETWORK;
  }
  LOGI("setupCoordinatorMode", "Mode is: %d", systemCfg.workMode);

  if (systemCfg.workMode != WORK_MODE_USB || systemCfg.keepWeb)
  { // start network overseer
    if (tmrNetworkOverseer.state() == STOPPED)
    {
      tmrNetworkOverseer.start();
    }
    WiFi.onEvent(NetworkEvent);
  }
  switch (systemCfg.workMode)
  {
  case WORK_MODE_USB:
    DEBUG_PRINTLN(F("Coordinator USB mode"));
    ledControl.modeLED.mode = LED_ON;
    delay(500);
    if (vars.hwUartSelIs)
    {
      digitalWrite(hwConfig.mist.uartSelPin, 1);
      DEBUG_PRINTLN(F("digitalWrite(hwConfig.mist.uartSelPin, 1) - HIGH"));
    }
    break;

    // case COORDINATOR_MODE_WIFI:
    // DEBUG_PRINTLN(F("Coordinator WIFI mode"));
    //  initLan();
    // connectWifi();
    // break;

  case WORK_MODE_NETWORK:
    DEBUG_PRINTLN(F("Coordinator NETWORK mode"));
    ledControl.powerLED.mode = LED_BLINK_1Hz;
    if (networkCfg.ethEnable)
      initLan();
    if (networkCfg.wifiEnable)
      connectWifi();
    break;

  default:
    break;
  }
  if (!systemCfg.disableWeb && (systemCfg.workMode != WORK_MODE_USB || systemCfg.keepWeb))
    updWeb = true; // handle web server
  if (systemCfg.workMode == WORK_MODE_USB && systemCfg.keepWeb)
    connectWifi(); // try 2 connect wifi
}

// void cmd2zigbee(const HardwareSerial serial, byte cmd[], const byte size){
//   byte checksum;
//   for (byte i = 1; i < size - 1; i++){
//     checksum ^= cmd[i];
//   }
//   cmd[size] = checksum;
//   serial.write(cmd, size);
// }

// void clearS2Buffer(){
//   while (Serial2.available()){//clear buffer
//     Serial2.read();
//   }
// }

void setup()
{
  Serial.begin(115200); // todo ifdef DEBUG

  String tag = "SETUP";

  initNVS();

  getDeviceID(vars.deviceId); // need for mqtt, vpn, mdns, wifi ap and so on

  loadSystemConfig(systemCfg);
  loadNetworkConfig(networkCfg);
  loadVpnConfig(vpnCfg);
  loadMqttConfig(mqttCfg);

  /*
  LOGI(tag, "After NVS load config\n");
  delay(500);
  printConfig(networkCfg, vpnCfg, mqttCfg, systemCfg);
  */

  // LOAD System vars and create FS / start
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
  {
    LOGI(tag, "Error with LITTLEFS");
    return;
  }

  // LOAD System vars and create FS / end

  // systemCfg.serialSpeed = 115200;

  loadFileConfigHW();

  vars.apStarted = false;

  // AVOID USING PIN 0
  if (hwConfig.mist.btnPin > 0)
  {
    pinMode(hwConfig.mist.btnPin, INPUT);
    vars.hwBtnIs = true;
  }

  if (hwConfig.mist.ledUsbPin > 0)
  {
    pinMode(hwConfig.mist.ledUsbPin, OUTPUT);
    vars.hwLedUsbIs = true;

    ledControl.modeLED.name = "Mode";
    ledControl.modeLED.pin = hwConfig.mist.ledUsbPin;
    ledControl.modeLED.active = true;
    ledControl.modeLED.mode = LED_OFF;

    LOGI(tag, "%d", ledControl.modeLED.mode);

    xTaskCreate(ledTask, "MODE LED Task", 2048, &ledControl.modeLED, 6, NULL);
  }

  if (hwConfig.mist.ledPwrPin > 0)
  {
    pinMode(hwConfig.mist.ledPwrPin, OUTPUT);
    vars.hwLedPwrIs = true;

    ledControl.powerLED.name = "Power";
    ledControl.powerLED.pin = hwConfig.mist.ledPwrPin;
    ledControl.powerLED.active = true;
    ledControl.powerLED.mode = LED_OFF;

    LOGI(tag, "%d", ledControl.powerLED.mode);

    xTaskCreate(ledTask, "PWR LED Task", 2048, &ledControl.powerLED, 6, NULL);
  }

  if (hwConfig.mist.uartSelPin > 0)
  {
    pinMode(hwConfig.mist.uartSelPin, OUTPUT);
    DEBUG_PRINTLN(F("pinMode(hwConfig.mist.uartSelPin, OUTPUT)"));
    digitalWrite(hwConfig.mist.uartSelPin, 0); // enable zigbee serial on LAN.so free USB
    DEBUG_PRINTLN(F("enable zigbee serial 1 - LOW"));
    vars.hwUartSelIs = true;
  }

  if ((hwConfig.zb.txPin > 0) && (hwConfig.zb.rxPin > 0) && (hwConfig.zb.rstPin > 0) && (hwConfig.zb.bslPin > 0))
  {
    if (vars.hwUartSelIs == true)
    {
      digitalWrite(hwConfig.mist.uartSelPin, 0); // enable zigbee serial
      DEBUG_PRINTLN(F("enable zigbee serial 2 - LOW"));
    }

    Serial2.begin(systemCfg.serialSpeed, SERIAL_8N1, hwConfig.zb.rxPin, hwConfig.zb.txPin); // start zigbee serial

    zbHwCheck();
  }

  if (vars.hwBtnIs)
  {
    // hard reset BTN
    // #if BUILD_ENV_NAME != debug
    if (!digitalRead(hwConfig.mist.btnPin))
    {
      DEBUG_PRINTLN(F("[hard reset] Entering hard reset mode"));
      uint8_t counter = 0;
      while (!digitalRead(hwConfig.mist.btnPin))
      {
        if (counter >= 10)
        {
          factoryReset();
        }
        else
        {
          counter++;
          DEBUG_PRINTLN(counter);
          delay(200);
        }
      }
      DEBUG_PRINTLN(F("[hard reset] Btn up, exit"));
    }
    // #endif

    attachInterrupt(digitalPinToInterrupt(hwConfig.mist.btnPin), btnInterrupt, FALLING);
  }

  // READ file to support migrate from old firmware
  loadFileSystemVar();
  loadFileConfigSerial();
  loadFileConfigWifi();
  loadFileConfigEther();
  loadFileConfigGeneral();
  loadFileConfigSecurity();
  loadFileConfigMqtt();
  loadFileConfigWg();
  // READ file to support migrate from old firmware

  LOGI(tag, "After full load config");
  // printConfig(networkCfg, vpnCfg, mqttCfg, systemCfg);
  String cfg = makeJsonConfig(&networkCfg, &vpnCfg, &mqttCfg, &systemCfg, &vars);
  DEBUG_PRINTLN(cfg);

  // DEBUG_PRINTLN("systemCfg.disableLeds: ");
  // DEBUG_PRINTLN(systemCfg.disableLeds);
  setLedsDisable(); // with setup ?? // move to vars ?

  setupCoordinatorMode();
  vars.connectedClients = 0;

  // initZb();

  // DEBUG_PRINTLN(millis());

  // Serial2.updateBaudRate(systemCfg.serialSpeed); // set actual speed
  printLogMsg("Setup done");
  // delay(2000);
  xTaskCreate(updateWebTask, "update Web Task", 2048, NULL, 7, NULL);

  printNVSFreeSpace();

  /*
  LOGI(tag, "Saving config to NVS");
  saveNetworkConfig(networkCfg);
  saveVpnConfig(vpnCfg);
  saveMqttConfig(mqttCfg);
  saveSystemConfig(systemCfg);
  printNVSFreeSpace();
  */

  // char deviceIdArr[MAX_DEV_ID_LONG];
  // getDeviceID(deviceIdArr);

  // DEBUG_PRINTLN(String(deviceIdArr));
  // printLogMsg(String(deviceIdArr));

  // Cron.create(const_cast<char *>("0 */1 * * * *"), ledsScheduler, false);

  /*
  cron_parse_expr(cronstring, &(Alarm[id].expr), &err);
  if (err) {
    memset(&(Alarm[id].expr), 0, sizeof(Alarm[id].expr));
    return dtINVALID_ALARM_ID;
  }
  */
}


WiFiClient client[10];

void socketClientConnected(int client)
{
  if (vars.connectedSocket[client] != true)
  {
    DEBUG_PRINT(F("Connected client "));
    DEBUG_PRINTLN(client);
    if (vars.connectedClients == 0)
    {
      vars.socketTime = millis();
      DEBUG_PRINT(F("Socket time "));
      DEBUG_PRINTLN(vars.socketTime);
      mqttPublishIo("socket", "ON");
      ledControl.powerLED.mode = LED_ON;
    }
    vars.connectedSocket[client] = true;
    vars.connectedClients++;
  }
}

void socketClientDisconnected(int client)
{
  if (vars.connectedSocket[client] != false)
  {
    DEBUG_PRINT(F("Disconnected client "));
    DEBUG_PRINTLN(client);
    vars.connectedSocket[client] = false;
    vars.connectedClients--;
    if (vars.connectedClients == 0)
    {
      vars.socketTime = millis();
      DEBUG_PRINT(F("Socket time "));
      DEBUG_PRINTLN(vars.socketTime);
      mqttPublishIo("socket", "OFF");
      ledControl.powerLED.mode = LED_BLINK_1Hz;
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
    // if (serial_buf[i] == 0x01)
    //{
    // }
    sprintf(output_sprintf, "%02x", serial_buf[i]);
    logPush(' ');
    logPush(output_sprintf[0]);
    logPush(output_sprintf[1]);
    // if (serial_buf[i] == 0x03)
    // {

    //}
  }
  logPush('\n');
}

void loop(void)
{
  if (btnFlag && vars.hwBtnIs)
  {
    if (!digitalRead(hwConfig.mist.btnPin))
    { // pressed
      if (tmrBtnLongPress.state() == STOPPED)
      {
        tmrBtnLongPress.start();
      }
    }
    // else
    //{
    /*if (tmrBtnLongPress.state() == RUNNING)
    {
      btnFlag = false;
      tmrBtnLongPress.stop();
      toggleUsbMode();
    }*/
    //}
  }

  tmrBtnLongPress.update();
  tmrNetworkOverseer.update();
  if (updWeb)
  {
    webServerHandleClient();
  }
  else
  {
    if (vars.connectedClients == 0)
    {
      webServerHandleClient();
    }
  }

  if (systemCfg.workMode != WORK_MODE_USB)
  {
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
          if (systemCfg.fwEnabled)
          {
            WiFiClient TempClient2 = server.available();
            if (TempClient2.remoteIP() == systemCfg.fwIp)
            {
              printLogMsg(String("[SOCK IP WHITELIST] Accepted connection from IP: ") + TempClient2.remoteIP().toString());
              client[i] = TempClient2;
              continue;
            }
            else
            {
              printLogMsg(String("[SOCK IP WHITELIST] Rejected connection from unknown IP: ") + TempClient2.remoteIP().toString());
            }
          }
          else
          {
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

    if (mqttCfg.enable)
    {
      mqttLoop();
    }
  }
  if (vpnCfg.wgEnable && vars.vpnWgInit)
  {
    wgLoop();
  }

  if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA)
  {
    dnsServer.processNextRequest();
  }
  Cron.delay();
}
