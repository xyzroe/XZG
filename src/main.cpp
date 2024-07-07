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

// NO SSL SUPPORT in current SDK
// #define ASYNC_TCP_SSL_ENABLED 1

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "mqtt.h"
#include "zb.h"
#include "version.h"
// #include "const/hw.h"
#include "per.h"

#include "esp_system.h"
#include "esp_task_wdt.h"

/*
#ifdef ETH_CLK_MODE
#undef ETH_CLK_MODE
#endif
*/

extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

LEDControl ledControl;

ThisConfigStruct hwConfig;

SystemConfigStruct systemCfg;
NetworkConfigStruct networkCfg;
VpnConfigStruct vpnCfg;
MqttConfigStruct mqttCfg;

SysVarsStruct vars;

extern int btnFlag;

bool updWeb = false;

void mDNS_start();
void connectWifi();
// void handleLongBtn();
void handleTmrNetworkOverseer();
void setupCoordinatorMode();
void startAP(const bool start);
// void toggleUsbMode();

Ticker tmrNetworkOverseer(handleTmrNetworkOverseer, overseerInterval, 0, MILLIS);

IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
WiFiServer server(ZB_TCP_PORT, MAX_SOCKET_CLIENTS);

CCTools CCTool(Serial2);

MDNSResponder mDNS;

void initLan()
{

  if (ETH.begin(hwConfig.eth.addr, hwConfig.eth.pwrPin, hwConfig.eth.mdcPin, hwConfig.eth.mdiPin, hwConfig.eth.phyType, hwConfig.eth.clkMode)) // hwConfig.eth.pwrAltPin))
  {
    String modeString = networkCfg.ethDhcp ? "DHCP" : "Static";
    LOGD("LAN start ok, %s", modeString);
    // ConfigSettings.disconnectEthTime = millis();
    if (!networkCfg.ethDhcp)
    {
      ETH.config(networkCfg.ethIp, networkCfg.ethGate, networkCfg.ethMask, networkCfg.ethDns1, networkCfg.ethDns2);
    }
  }
  else
  {
    LOGD("LAN start err");
    // esp_eth_stop();
  }
}

void startSocketServer()
{
  server.begin(systemCfg.socketPort);
  server.setNoDelay(true);
}

void startServers(bool usb = false)
{

  if (!vars.apStarted)
  {
    xTaskCreate(setClock, "setClock", 2048, NULL, 9, NULL);
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
  }

  initWebServer();

  startAP(false);

  /*if (!vars.apStarted)
  {
    if (vpnCfg.wgEnable)
    {
      wgBegin();
    }
  }*/

  mDNS_start();
  /* //not available now
  if (vpnCfg.hnEnable)
  {
    hnBegin();
  }
  */
}

void handleTmrNetworkOverseer()
{
  // switch (systemCfg.workMode)
  //{
  // case WORK_MODE_NETWORK:
  if (!networkCfg.wifiEnable && !networkCfg.ethEnable)
  {
    if (!vars.apStarted)
    {
      LOGD("Both interfaces disabled. Start AP");
      startAP(true);
      connectWifi();
    }
  }
  if (networkCfg.wifiEnable)
  {
    LOGD("WiFi.status = %s", String(WiFi.status()));

    if (WiFi.isConnected())
    {
      LOGD("WIFI CONNECTED");
      tmrNetworkOverseer.stop();
      if (!vars.firstUpdCheck)
      {
        checkUpdateAvail();
        vars.firstUpdCheck = true;
      }
    }
    else
    {
      if (tmrNetworkOverseer.counter() > overseerMaxRetry)
      {
        LOGD("WIFI counter overflow");
        startAP(true);
        connectWifi();
      }
    }
  }
  if (networkCfg.ethEnable)
  {
    if (vars.connectedEther)
    {
      LOGD("LAN CONNECTED");
      tmrNetworkOverseer.stop();
      if (vars.apStarted)
      {
        startAP(false);
      }
      if (!vars.firstUpdCheck)
      {
        checkUpdateAvail();
        vars.firstUpdCheck = true;
      }
    }
    else
    {
      if (tmrNetworkOverseer.counter() > overseerMaxRetry)
      {
        LOGD("LAN counter overflow");
        startAP(true);
      }
    }
  }
  // break;
  /*case WORK_MODE_USB:
    if (tmrNetworkOverseer.counter() > 3)
    { // 10 seconds for wifi connect
      if (WiFi.isConnected())
      {
        tmrNetworkOverseer.stop();
        // startServers(true);
      }
      else
      {
        initLan();
        if (tmrNetworkOverseer.counter() > 6)
        { // 3sec for lan
          if (vars.connectedEther)
          {
            tmrNetworkOverseer.stop();
            // startServers(true);
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
  default:
    break;*/
  //}
}

void NetworkEvent(WiFiEvent_t event)
{
  const char *wifiKey = "WiFi";
  const char *ethKey = "ETH";
  //esp_err_t result5;
  switch (event)
  {
  case ARDUINO_EVENT_ETH_START: // 18: // SYSTEM_EVENT_ETH_START:
    LOGD("%s Started", ethKey);
    //  ConfigSettings.disconnectEthTime = millis();
    ETH.setHostname(systemCfg.hostname);
    break;
  case ARDUINO_EVENT_ETH_CONNECTED: // 20: // SYSTEM_EVENT_ETH_CONNECTED:
    LOGD("%s Connected", ethKey);
    break;
  case ARDUINO_EVENT_ETH_GOT_IP: // 22: // SYSTEM_EVENT_ETH_GOT_IP:
    // startServers();
    LOGI("%s MAC: %s, IP: %s, Mask: %s, Gw: %s, DNS: %s, %dMbps", ethKey,
         ETH.macAddress().c_str(),
         ETH.localIP().toString().c_str(),
         ETH.subnetMask().toString().c_str(),
         ETH.gatewayIP().toString().c_str(),
         ETH.dnsIP().toString().c_str(),
         ETH.linkSpeed());

    vars.connectedEther = true;
    checkDNS(true);
    // ConfigSettings.disconnectEthTime = 0;
    break;
  case ARDUINO_EVENT_ETH_DISCONNECTED: // 21:  //SYSTEM_EVENT_ETH_DISCONNECTED:
    LOGD("%s Disconnected", ethKey);
    vars.connectedEther = false;
    // ConfigSettings.disconnectEthTime = millis();
    if (tmrNetworkOverseer.state() == STOPPED) //&& systemCfg.workMode == WORK_MODE_NETWORK)
    {
      tmrNetworkOverseer.start();
    }
    break;
  case SYSTEM_EVENT_ETH_STOP: // 27:
  case ARDUINO_EVENT_ETH_STOP:
    LOGD("%s Stopped", ethKey);
    vars.connectedEther = false;
    // ConfigSettings.disconnectEthTime = millis();
    if (tmrNetworkOverseer.state() == STOPPED)
    {
      tmrNetworkOverseer.start();
    }
    break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP: // SYSTEM_EVENT_STA_GOT_IP:
    // startServers();
    LOGI("%s MAC: %s, IP: %s, Mask: %s, Gw: %s, DNS: %s", wifiKey,
         WiFi.macAddress().c_str(),
         WiFi.localIP().toString().c_str(),
         WiFi.subnetMask().toString().c_str(),
         WiFi.gatewayIP().toString().c_str(),
         WiFi.dnsIP().toString().c_str());
    checkDNS(true);
    LOGD("WiFi TX %s", String(WiFi.getTxPower()));

    /*result5 = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    if (result5 == ESP_OK)
    {
      Serial.println("Wi-Fi protocol set successfully.");
    }
    else
    {
      Serial.printf("Error setting Wi-Fi protocol: %d\n", result5);
    }

    uint8_t cur_mode;
    esp_wifi_get_protocol(WIFI_IF_STA, &cur_mode);
    Serial.print("Current Wi-Fi protocol: ");
    if (cur_mode & WIFI_PROTOCOL_11B)
      Serial.print("802.11b ");
    if (cur_mode & WIFI_PROTOCOL_11G)
      Serial.print("802.11g ");
    if (cur_mode & WIFI_PROTOCOL_11N)
      Serial.print("802.11n ");
    Serial.println();*/
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED: // SYSTEM_EVENT_STA_DISCONNECTED:
    LOGD("%s STA DISCONNECTED", wifiKey);
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
  LOGD("begin cmd=%d, state=%d", start, vars.apStarted);

  if (vars.apStarted)
  {
    if (!start)
    {
      if (!networkCfg.wifiEnable)
      {
        WiFi.softAPdisconnect(true); // off wifi
      }
      else
      {
        WiFi.mode(WIFI_STA);
      }
      dnsServer.stop();
      vars.apStarted = false;
    }
  }
  else
  {
    if (!start)
      return;
    LOGD("WIFI_AP_STA");
    WiFi.mode(WIFI_AP_STA); // WIFI_AP_STA for possible wifi scan in wifi mode
    WiFi.disconnect();
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(vars.deviceId); //, WIFIPASS);

    // if DNSServer is started with "*" for domain name, it will reply with
    // provided IP to all DNS request
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", apIP);
    WiFi.setSleep(false);
    // ConfigSettings.wifiAPenblTime = millis();
    LOGD("startServers()");
    startServers();
    vars.apStarted = true;
  }
}

void connectWifi()
{
  static uint8_t timeout = 0;
  if (WiFi.status() == WL_IDLE_STATUS && timeout < 20)
  { // connection in progress
    LOGD("WL_IDLE_STATUS");
    timeout++;
    return;
  }
  else
  {
    timeout = 0;
    LOGD("timeout");
  }
  WiFi.persistent(false);

  /*uint8_t cur_mode;
  esp_wifi_get_protocol(WIFI_IF_STA, &cur_mode);
  Serial.print("wifi mode ");
  String result = "";
  result += String(cur_mode, DEC);
  Serial.println(result);

  esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N); // networkCfg.wifiMode); // WIFI_PROTOCOL_11B | ); //*/

  if ((strlen(networkCfg.wifiSsid) >= 2) && (strlen(networkCfg.wifiPass) >= 8))
  {
    LOGD("Ok SSID & PASS");
    if (vars.apStarted)
    {
      // LOGD("WiFi.mode(WIFI_AP_STA)");
      // WiFi.mode(WIFI_AP_STA);
    }
    else
    {
      WiFi.setHostname(systemCfg.hostname);
      LOGD("WiFi.mode(WIFI_STA)");
      WiFi.mode(WIFI_STA);
    }
    delay(100);

    WiFi.setSleep(false);

    if (!networkCfg.wifiDhcp)
    {
      WiFi.config(networkCfg.wifiIp, networkCfg.wifiGate, networkCfg.wifiMask, networkCfg.wifiDns1, networkCfg.wifiDns2);
      LOGD("WiFi.config");
    }
    else
    {
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
      LOGD("Try DHCP");
    }
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
    WiFi.begin(networkCfg.wifiSsid, networkCfg.wifiPass);
    WiFi.setAutoReconnect(true);
    WiFi.setTxPower(networkCfg.wifiPower);
    LOGD("WiFi TX %s", String(WiFi.getTxPower()));
    /*esp_err_t result = esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    if (result == ESP_OK)
    {
      Serial.println("Wi-Fi protocol set successfully.");
    }
    else
    {
      Serial.printf("Error setting Wi-Fi protocol: %d\n", result);
    }*/
    LOGD("WiFi.begin");
  }
  else
  {
    // if (!(systemCfg.workMode == WORK_MODE_USB && systemCfg.keepWeb))
    //{ // dont start ap in keepWeb
    LOGD("NO SSID & PASS ");
    if (!vars.connectedEther)
    {
      LOGD("and problem with LAN");
      startAP(true);
      LOGD("so setupWifiAP");
    }
    else
    {
      LOGD("but LAN is OK");
    }
    // }
  }
}

void mDNS_start()
{
  const char *host = "_xzg";
  const char *http = "_http";
  const char *tcp = "_tcp";
  if (!mDNS.begin(systemCfg.hostname))
  {
    LOGD("Error setting up mDNS responder!");
  }
  else
  {
    LOGI("mDNS responder started on %s.local", String(systemCfg.hostname));
    //----- WEB ------
    mDNS.addService(http, tcp, 80);
    //--zeroconf zha--
    mDNS.addService(host, tcp, systemCfg.socketPort);
    mDNS.addServiceTxt(host, tcp, "version", "1.0");
    mDNS.addServiceTxt(host, tcp, "radio_type", "znp");
    mDNS.addServiceTxt(host, tcp, "baud_rate", String(systemCfg.serialSpeed));
    mDNS.addServiceTxt(host, tcp, "data_flow_control", "software");
    mDNS.addServiceTxt(host, tcp, "board", String(hwConfig.board));
  }
}

void setupCoordinatorMode()
{
  if (systemCfg.workMode > 2 || systemCfg.workMode < 0)
  {
    LOGW("WRONG MODE, set to Network");
    systemCfg.workMode = WORK_MODE_NETWORK;
  }

  String workModeString = systemCfg.workMode ? "USB" : "Network";
  LOGI("%s", workModeString);

  // if ((systemCfg.workMode != WORK_MODE_USB) || systemCfg.keepWeb)
  //{ // start network overseer
  if (tmrNetworkOverseer.state() == STOPPED)
  {
    tmrNetworkOverseer.start();
  }
  WiFi.onEvent(NetworkEvent);
  if (networkCfg.ethEnable)
    initLan();
  if (networkCfg.wifiEnable)
    connectWifi();
  //}

  switch (systemCfg.workMode)
  {
  case WORK_MODE_USB:
    ledControl.modeLED.mode = LED_ON;
    delay(100);
    usbModeSet(ZIGBEE);
    startServers(true);
    break;
  case WORK_MODE_NETWORK:
    ledControl.powerLED.mode = LED_BLINK_1Hz;
    delay(100);
    usbModeSet(XZG);
    startServers();
    break;
  default:
    break;
  }

  // if (!systemCfg.disableWeb && ((systemCfg.workMode != WORK_MODE_USB) || systemCfg.keepWeb))
  //   updWeb = true; // handle web server
  if (!systemCfg.disableWeb)
    updWeb = true; // handle web server
  // if (systemCfg.workMode == WORK_MODE_USB && systemCfg.keepWeb)
  //   connectWifi(); // try 2 connect wifi
}

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

  // LOAD System vars and create FS / start
  if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED, "/lfs2", 10))
  {
    LOGD("Error with LITTLEFS");
    return;
  }

  // LOAD System vars and create FS / end

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

  // String cfg = makeJsonConfig(&networkCfg, &vpnCfg, &mqttCfg, &systemCfg, &vars);
  // LOGD("After READ OLD config\n %s", cfg.c_str());

  loadFileConfigHW();

  if (hwConfig.eth.mdcPin == -1 || hwConfig.eth.mdiPin == -1)
  {
    if (networkCfg.ethEnable)
    {
      networkCfg.ethEnable = false;
      saveNetworkConfig(networkCfg);
    }
  }

  vars.apStarted = false;

  // AVOID USING PIN 0
  if (hwConfig.mist.btnPin > 0)
  {
    buttonInit();
  }

  if (hwConfig.mist.ledModePin > 0)
  {
    ledModeSetup();
  }

  if (hwConfig.mist.ledPwrPin > 0)
  {
    ledPwrSetup();
  }

  if (hwConfig.mist.uartSelPin > 0)
  {
    pinMode(hwConfig.mist.uartSelPin, OUTPUT);
    // vars.hwUartSelIs = true;
    //  usbModeSet(XZG);
    bool fixState = (hwConfig.mist.uartSelPlr == 1) ? LOW : HIGH;
    digitalWrite(hwConfig.mist.uartSelPin, fixState);
  }

  if ((hwConfig.zb.txPin > 0) && (hwConfig.zb.rxPin > 0) && (hwConfig.zb.rstPin > 0) && (hwConfig.zb.bslPin > 0))
  {
    Serial2.begin(systemCfg.serialSpeed, SERIAL_8N1, hwConfig.zb.rxPin, hwConfig.zb.txPin); // start zigbee serial
    int BSL_PIN_MODE = 0;
    if (CCTool.begin(hwConfig.zb.rstPin, hwConfig.zb.bslPin, BSL_PIN_MODE))
    {
      zbHwCheck();
    }
  }

  if (vars.hwBtnIs)
  {
    buttonSetup();
  }

  String cfg = makeJsonConfig(&networkCfg, &vpnCfg, &mqttCfg, &systemCfg);
  LOGI("Config:\n%s", cfg.c_str());

  cfg = makeJsonConfig(NULL, NULL, NULL, NULL, &vars);
  LOGI("VARS:\n%s", cfg.c_str());

  setLedsDisable(); // with setup ?? // move to vars ?

  setupCoordinatorMode();
  vars.connectedClients = 0;

  xTaskCreate(updateWebTask, "update Web Task", 2048, NULL, 8, NULL);

  printNVSFreeSpace();

  if (systemCfg.zbRole == COORDINATOR || systemCfg.zbRole == UNDEFINED)
  {
    if (zbFwCheck())
    {
      if (systemCfg.zbRole == UNDEFINED)
      {
        systemCfg.zbRole = COORDINATOR;
        saveSystemConfig(systemCfg);
      }
    }
  }
  else
  {
    LOGI("[ZB] role: %s", String(systemCfg.zbRole));
  }
  LOGI("[ESP] FW: %s", String(VERSION));

  LOGI("Load cfg %s", hwConfig.board);

  setup1wire(check1wire());

  LOGI("done");
}

WiFiClient client[10];

void socketClientConnected(int client, IPAddress ip)
{
  if (vars.connectedSocket[client] != true)
  {
    printLogMsg("Connected client " + String(client + 1) + " from " + ip.toString().c_str());
    if (vars.connectedClients == 0)
    {
      vars.socketTime = millis();
      mqttPublishIo("socket", 1);
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
    LOGD("Disconnected client %d", client);
    vars.connectedSocket[client] = false;
    vars.connectedClients--;
    if (vars.connectedClients == 0)
    {
      vars.socketTime = millis();
      mqttPublishIo("socket", 0);
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
    buttonLoop();
  }

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

  if (!vars.zbFlashing)
  {

    if (systemCfg.workMode == WORK_MODE_USB)
    {
      if (Serial2.available())
      {
        Serial.write(Serial2.read());
        Serial.flush();
      }
      if (Serial.available())
      {
        Serial2.write(Serial.read());
        Serial2.flush();
      }
      return;
    }

    else if (systemCfg.workMode == WORK_MODE_NETWORK)
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
          socketClientConnected(cln, client[cln].remoteIP());
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

      /*if (mqttCfg.enable)
      {
        // mqttLoop();
      }*/
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
}
