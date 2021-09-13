#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include "WiFi.h"
#include <WebServer.h>
#include "FS.h"
#include "LITTLEFS.h"
#include "web.h"
#include "config.h"
#include "log.h"
#include "etc.h"
#include <Update.h>
#include "html.h"

extern struct ConfigSettingsStruct ConfigSettings;
extern unsigned long timeLog;

WebServer serverWeb(80);

void webServerHandleClient()
{
  serverWeb.handleClient();
}

void initWebServer()
{
  serverWeb.serveStatic("/web/js/jquery-min.js", LITTLEFS, "/web/js/jquery-min.js");
  serverWeb.serveStatic("/web/js/functions.js", LITTLEFS, "/web/js/functions.js");
  serverWeb.serveStatic("/web/js/bootstrap.min.js", LITTLEFS, "/web/js/bootstrap.min.js");
  serverWeb.serveStatic("/web/css/bootstrap.min.css", LITTLEFS, "/web/css/bootstrap.min.css");
  serverWeb.serveStatic("/web/css/glyphicons.css", LITTLEFS, "/web/css/glyphicons.css");
  serverWeb.serveStatic("/web/fonts/glyphicons.woff", LITTLEFS, "/web/fonts/glyphicons.woff");
  serverWeb.serveStatic("/web/css/style.css", LITTLEFS, "/web/css/style.css");
  serverWeb.serveStatic("/web/img/logo.png", LITTLEFS, "/web/img/logo.png");
  serverWeb.serveStatic("/web/img/wait.gif", LITTLEFS, "/web/img/wait.gif");
  serverWeb.serveStatic("/web/img/nok.png", LITTLEFS, "/web/img/nok.png");
  serverWeb.serveStatic("/web/img/ok.png", LITTLEFS, "/web/img/ok.png");
  serverWeb.on("/", handleRoot);
  serverWeb.on("/general", handleGeneral);
  serverWeb.on("/wifi", handleWifi);
  serverWeb.on("/ethernet", handleEther);
  serverWeb.on("/serial", handleSerial);
  serverWeb.on("/saveGeneral", HTTP_POST, handleSaveGeneral);
  serverWeb.on("/saveSerial", HTTP_POST, handleSaveSerial);
  serverWeb.on("/saveWifi", HTTP_POST, handleSaveWifi);
  serverWeb.on("/saveEther", HTTP_POST, handleSaveEther);
  serverWeb.on("/fsbrowser", handleFSbrowser);
  serverWeb.on("/logs", handleLogs);
  serverWeb.on("/reboot", handleReboot);
  serverWeb.on("/updates", handleUpdate);
  serverWeb.on("/readFile", handleReadfile);
  serverWeb.on("/saveFile", handleSavefile);
  serverWeb.on("/getLogBuffer", handleLogBuffer);
  serverWeb.on("/scanNetwork", handleScanNetwork);
  serverWeb.on("/cmdClearConsole", handleClearConsole);
  serverWeb.on("/cmdGetVersion", handleGetVersion);
  serverWeb.on("/cmdZigRST", handleZigbeeReset);
  serverWeb.on("/cmdZigBSL", handleZigbeeBSL);
  serverWeb.on("/switch/firmware_update/toggle", handleZigbeeBSL); //for cc-2538.py ESPHome edition back compatibility
  serverWeb.on("/help", handleHelp);
  serverWeb.on("/esp_update", handleESPUpdate);
  serverWeb.onNotFound(handleNotFound);
  /*handling uploading firmware file */
  serverWeb.on(
      "/update", HTTP_POST, []()
      {
        serverWeb.sendHeader("Connection", "close");
        serverWeb.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
      },
      []()
      {
        HTTPUpload &upload = serverWeb.upload();
        if (upload.status == UPLOAD_FILE_START)
        {
          Serial.printf("Update: %s\n", upload.filename.c_str());
          if (!Update.begin(UPDATE_SIZE_UNKNOWN))
          { //start with max available size
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_WRITE)
        {
          /* flashing firmware to ESP*/
          if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
          {
            Update.printError(Serial);
          }
        }
        else if (upload.status == UPLOAD_FILE_END)
        {
          if (Update.end(true))
          { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
          }
          else
          {
            Update.printError(Serial);
          }
        }
      });
  serverWeb.begin();
}

void handleNotFound()
{

  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += serverWeb.uri();
  message += F("\nMethod: ");
  message += (serverWeb.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += serverWeb.args();
  message += F("\n");

  for (uint8_t i = 0; i < serverWeb.args(); i++)
  {
    message += " " + serverWeb.argName(i) + ": " + serverWeb.arg(i) + "\n";
  }

  serverWeb.send(404, F("text/plain"), message);
}

void handleHelp()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_HELP);
  result += F("</html>");

  serverWeb.send(200, "text/html", result);
}

void handleGeneral()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_GENERAL);
  result += F("</html>");

  if (ConfigSettings.disableWeb)
  {
    result.replace("{{disableWeb}}", "checked");
  }
  else
  {
    result.replace("{{disableWeb}}", "");
  }
  if (ConfigSettings.enableHeartBeat)
  {
    result.replace("{{enableHeartBeat}}", "checked");
  }
  else
  {
    result.replace("{{enableHeartBeat}}", "");
  }

  result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);
  result.replace("{{hostname}}", (String)ConfigSettings.hostname);

  serverWeb.send(200, "text/html", result);
}

void handleSaveSucces(String msg)
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h2>Saved</h2>");
  result += F("<div class='row justify-content-md-center'>");
  result += F("<div class='col-sm-6'>");
  result += F("<form method='GET' action='reboot' id='upload_form'>");
  result += F("<label>Save ");
  result += msg;
  result += F(" OK !</label><br><br><br>");
  result += F("<button type='submit' class='btn btn-warning mb-2'>Reboot</button>");
  result += F("</form></div></div>");
  result += F("</html>");
  serverWeb.send(200, "text/html", result);
}

void handleWifi()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_WIFI);
  result += F("</html>");

  DEBUG_PRINTLN(ConfigSettings.enableWiFi);
  if (ConfigSettings.enableWiFi)
  {

    result.replace("{{checkedWiFi}}", "Checked");
  }
  else
  {
    result.replace("{{checkedWiFi}}", "");
  }
  result.replace("{{ssid}}", String(ConfigSettings.ssid));
  if (ConfigSettings.dhcpWiFi)
  {
    result.replace("{{modeWiFi}}", "Checked");
  }
  else
  {
    result.replace("{{modeWiFi}}", "");
  }
  result.replace("{{ip}}", ConfigSettings.ipAddressWiFi);
  result.replace("{{mask}}", ConfigSettings.ipMaskWiFi);
  result.replace("{{gw}}", ConfigSettings.ipGWWiFi);

  serverWeb.send(200, "text/html", result);
}

void handleSerial()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_SERIAL);
  result += F("</html>");
  if (ConfigSettings.serialSpeed == 9600)
  {
    result.replace("{{selected9600}}", "Selected");
  }
  else if (ConfigSettings.serialSpeed == 19200)
  {
    result.replace("{{selected19200}}", "Selected");
  }
  else if (ConfigSettings.serialSpeed == 38400)
  {
    result.replace("{{selected38400}}", "Selected");
  }
  else if (ConfigSettings.serialSpeed == 57600)
  {
    result.replace("{{selected57600}}", "Selected");
  }
  else if (ConfigSettings.serialSpeed == 115200)
  {
    result.replace("{{selected115200}}", "Selected");
  }
  else
  {
    result.replace("{{selected115200}}", "Selected");
  }
  result.replace("{{socketPort}}", String(ConfigSettings.socketPort));

  serverWeb.send(200, "text/html", result);
}

void handleEther()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_ETHERNET);
  result += F("</html>");

  if (ConfigSettings.dhcp)
  {
    result.replace("{{modeEther}}", "Checked");
  }
  else
  {
    result.replace("{{modeEther}}", "");
  }
  result.replace("{{ipEther}}", ConfigSettings.ipAddress);
  result.replace("{{maskEther}}", ConfigSettings.ipMask);
  result.replace("{{GWEther}}", ConfigSettings.ipGW);

  serverWeb.send(200, "text/html", result);
}

void handleRoot()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += FPSTR(HTTP_ROOT);
  result += F("</html>");

  String CPUtemp;
  getCPUtemp(CPUtemp);
  result.replace("{{deviceTemp}}", CPUtemp);

  if (ConfigSettings.enableWiFi)
  {
    result.replace("{{enableWifi}}", "<img src='/web/img/ok.png'>");
  }
  else
  {
    result.replace("{{enableWifi}}", "<img src='/web/img/nok.png'>");
  }
  result.replace("{{ssidWifi}}", String(ConfigSettings.ssid));

  if (ConfigSettings.dhcpWiFi)
  {
    result.replace("{{modeWiFi}}", "DHCP");
    result.replace("{{ipWifi}}", WiFi.localIP().toString());
    result.replace("{{maskWifi}}", WiFi.subnetMask().toString());
    result.replace("{{GWWifi}}", WiFi.gatewayIP().toString());
  }
  else
  {
    result.replace("{{modeWiFi}}", "STATIC");
    result.replace("{{ipWifi}}", ConfigSettings.ipAddressWiFi);
    result.replace("{{maskWifi}}", ConfigSettings.ipMaskWiFi);
    result.replace("{{GWWifi}}", ConfigSettings.ipGWWiFi);
  }

  result.replace("{{MACWifi}}", WiFi.softAPmacAddress());
  int rssi = WiFi.RSSI();
  String rssiWifi = String(rssi) + String(" dBm");
  result.replace("{{RSSIWifi}}", rssiWifi);
  result.replace("{{MACEther}}", ETH.macAddress());
  int speed = ETH.linkSpeed();
  String SpeedEth = String(speed) + String(" Mbps");
  if (ETH.fullDuplex())
  {
    SpeedEth = SpeedEth + String(", FULL DUPLEX");
  }
  else
  {
    SpeedEth = SpeedEth + String(", HALF DUPLEX");
  }
  result.replace("{{SpeedEther}}", SpeedEth);

  if (ConfigSettings.dhcp)
  {
    result.replace("{{modeEther}}", "DHCP");
    result.replace("{{ipEther}}", ETH.localIP().toString());
    result.replace("{{maskEther}}", ETH.subnetMask().toString());
    result.replace("{{GWEther}}", ETH.gatewayIP().toString());
  }
  else
  {
    result.replace("{{modeEther}}", "STATIC");
    result.replace("{{ipEther}}", ConfigSettings.ipAddress);
    result.replace("{{maskEther}}", ConfigSettings.ipMask);
    result.replace("{{GWEther}}", ConfigSettings.ipGW);
  }
  if (ConfigSettings.connectedEther)
  {
    result.replace("{{connectedEther}}", "<img src='/web/img/ok.png'>");
  }
  else
  {
    result.replace("{{connectedEther}}", "<img src='/web/img/nok.png'>");
  }

  if (ConfigSettings.radioModeWiFi)
  {
    result.replace("{{radioMode}}", "<img src='/web/img/ok.png'>");
  }
  else
  {
    result.replace("{{radioMode}}", "<img src='/web/img/nok.png'>");
  }

  if (ConfigSettings.connectedSocket)
  {
    result.replace("{{connectedSocket}}", "<img src='/web/img/ok.png'>");
  }
  else
  {
    result.replace("{{connectedSocket}}", "<img src='/web/img/nok.png'>");
  }

  String readableTime;
  getReadableTime(readableTime);
  result.replace("{{uptime}}", readableTime);

  serverWeb.send(200, "text/html", result);
}

void handleSaveGeneral()
{
  String StringConfig;
  String disableWeb;
  String enableHeartBeat;
  String refreshLogs;

  if (serverWeb.arg("disableWeb") == "on")
  {
    disableWeb = "1";
  }
  else
  {
    disableWeb = "0";
  }

  if (serverWeb.arg("enableHeartBeat") == "on")
  {
    enableHeartBeat = "1";
  }
  else
  {
    enableHeartBeat = "0";
  }

  if (serverWeb.arg("refreshLogs").toDouble() < 1000)
  {
    refreshLogs = "1000";
  }
  else
  {
    refreshLogs = serverWeb.arg("refreshLogs");
  }

  String hostname = serverWeb.arg("hostname");
  DEBUG_PRINTLN(hostname);
  const char *path = "/config/configGeneral.json";

  StringConfig = "{\"disableWeb\":" + disableWeb + ",\"enableHeartBeat\":" + enableHeartBeat + ",\"refreshLogs\":" + refreshLogs + ",\"hostname\":\"" + hostname + "\"}";
  DEBUG_PRINTLN(StringConfig);
  StaticJsonDocument<512> jsonBuffer;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LITTLEFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open"));
  }
  else
  {
    serializeJson(doc, configFile);
  }
  handleSaveSucces("config");
}

void handleSaveWifi()
{
  if (!serverWeb.hasArg("WIFISSID"))
  {
    serverWeb.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String StringConfig;
  String enableWiFi;
  if (serverWeb.arg("wifiEnable") == "on")
  {
    enableWiFi = "1";
  }
  else
  {
    enableWiFi = "0";
  }
  String ssid = serverWeb.arg("WIFISSID");
  String pass = serverWeb.arg("WIFIpassword");

  String dhcpWiFi;
  if (serverWeb.arg("dhcpWiFi") == "on")
  {
    dhcpWiFi = "1";
  }
  else
  {
    dhcpWiFi = "0";
  }

  String ipAddress = serverWeb.arg("ipAddress");
  String ipMask = serverWeb.arg("ipMask");
  String ipGW = serverWeb.arg("ipGW");

  const char *path = "/config/configWifi.json";

  StringConfig = "{\"enableWiFi\":" + enableWiFi + ",\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"dhcpWiFi\":" + dhcpWiFi + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
  DEBUG_PRINTLN(StringConfig);
  StaticJsonDocument<512> jsonBuffer;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LITTLEFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open"));
  }
  else
  {
    serializeJson(doc, configFile);
  }
  handleSaveSucces("config");
}

void handleSaveSerial()
{
  String StringConfig;
  String serialSpeed = serverWeb.arg("baud");
  String socketPort = serverWeb.arg("port");
  const char *path = "/config/configSerial.json";

  StringConfig = "{\"baud\":" + serialSpeed + ", \"port\":" + socketPort + "}";
  DEBUG_PRINTLN(StringConfig);
  StaticJsonDocument<512> jsonBuffer;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LITTLEFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open"));
  }
  else
  {
    serializeJson(doc, configFile);
  }
  handleSaveSucces("config");
}

void handleSaveEther()
{
  if (!serverWeb.hasArg("ipAddress"))
  {
    serverWeb.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String StringConfig;
  String dhcp;
  if (serverWeb.arg("dhcp") == "on")
  {
    dhcp = "1";
  }
  else
  {
    dhcp = "0";
  }
  String ipAddress = serverWeb.arg("ipAddress");
  String ipMask = serverWeb.arg("ipMask");
  String ipGW = serverWeb.arg("ipGW");

  const char *path = "/config/configEther.json";

  StringConfig = "{\"dhcp\":" + dhcp + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
  DEBUG_PRINTLN(StringConfig);
  StaticJsonDocument<512> jsonBuffer;
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, StringConfig);

  File configFile = LITTLEFS.open(path, FILE_WRITE);
  if (!configFile)
  {
    DEBUG_PRINTLN(F("failed open"));
  }
  else
  {
    serializeJson(doc, configFile);
  }
  handleSaveSucces("config");
}

void handleLogs()
{
  String result;

  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h2>Console</h2>");
  result += F("<div class='row justify-content-md-center'>");
  result += F("<div class='col-sm-6'>");
  result += F("<button type='button' onclick='cmd(\"ClearConsole\");document.getElementById(\"console\").value=\"\";' class='btn btn-primary'>Clear Console</button> ");
  result += F("<button type='button' onclick='cmd(\"GetVersion\");' class='btn btn-primary'>Get Version</button> ");
  //result += F("<button type='button' onclick='cmd(\"ErasePDM\");' class='btn btn-primary'>Erase PDM</button> ");
  result += F("<button type='button' onclick='cmd(\"ZigRST\");' class='btn btn-primary'>Zigbee Reset</button> ");
  result += F("<button type='button' onclick='cmd(\"ZigBSL\");' class='btn btn-primary'>Zigbee BSL</button> ");
  result += F("</div></div>");
  result += F("<div class='row justify-content-md-center' >");
  result += F("<div class='col-sm-6'>");

  result += F("Raw datas : <textarea id='console' rows='16' cols='100'>");

  result += F("</textarea></div></div>");
  //result += F("</div>");
  result += F("</body>");
  result += F("<script language='javascript'>");
  result += F("logRefresh({{refreshLogs}});");
  result += F("</script>");
  result += F("</html>");

  result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);

  serverWeb.send(200, F("text/html"), result);
}

void handleReboot()
{
  String result;

  result += F("<html>");
  result += F("<meta http-equiv='refresh' content='1; URL=/'>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h2>Rebooted</h2>");
  result = result + F("</body></html>");

  serverWeb.send(200, F("text/html"), result);

  ESP.restart();
}

void handleUpdate()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h2>Update Zigbee</h2>");
  result += F("<div class='btn-group-vertical'>");
  result += F("<a href='/setchipid' class='btn btn-primary mb-2'>setChipId</button>");
  result += F("<a href='/setmodeprod' class='btn btn-primary mb-2'>setModeProd</button>");
  result += F("</div>");

  result = result + F("</body></html>");

  serverWeb.send(200, F("text/html"), result);
}

void handleESPUpdate()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h2>Update ESP32</h2>");
  result += FPSTR(HTTP_UPDATE);
  result = result + F("</body></html>");
  serverWeb.sendHeader("Connection", "close");
  serverWeb.send(200, "text/html", result);
}

void handleFSbrowser()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h2>FSBrowser</h2>");
  result += F("<nav id='navbar-custom' class='navbar navbar-default navbar-fixed-left'>");
  result += F("      <div class='navbar-header'>");
  result += F("        <!--<a class='navbar-brand' href='#'>Brand</a>-->");
  result += F("      </div>");
  result += F("<ul class='nav navbar-nav'>");

  String str = "";
  File root = LITTLEFS.open("/config");
  File file = root.openNextFile();
  while (file)
  {
    String tmp = file.name();
    tmp = tmp.substring(8);
    result += F("<li><a href='#' onClick=\"readfile('");
    result += tmp;
    result += F("');\">");
    result += tmp;
    result += F(" ( ");
    result += file.size();
    result += F(" o)</a></li>");
    file = root.openNextFile();
  }
  result += F("</ul></nav>");
  result += F("<div class='container-fluid' >");
  result += F("  <div class='app-main-content'>");
  result += F("<form method='POST' action='saveFile'>");
  result += F("<div class='form-group'>");
  result += F(" <label for='file'>File : <span id='title'></span></label>");
  result += F("<input type='hidden' name='filename' id='filename' value=''>");
  result += F(" <textarea class='form-control' id='file' name='file' rows='10'>");
  result += F("</textarea>");
  result += F("</div>");
  result += F("<button type='submit' class='btn btn-primary mb-2'>Save</button>");
  result += F("</Form>");
  result += F("</div>");
  result += F("</div>");
  result += F("</body></html>");

  serverWeb.send(200, F("text/html"), result);
}

void handleReadfile()
{
  String result;
  String filename = "/config/" + serverWeb.arg(0);
  File file = LITTLEFS.open(filename, "r");

  if (!file)
  {
    return;
  }

  while (file.available())
  {
    result += (char)file.read();
  }
  file.close();
  serverWeb.send(200, F("text/html"), result);
}

void handleSavefile()
{
  if (serverWeb.method() != HTTP_POST)
  {
    serverWeb.send(405, F("text/plain"), F("Method Not Allowed"));
  }
  else
  {
    String filename = "/config/" + serverWeb.arg(0);
    String content = serverWeb.arg(1);
    File file = LITTLEFS.open(filename, "w");
    if (!file)
    {
      DEBUG_PRINT(F("Failed to open file for reading\r\n"));
      return;
    }

    int bytesWritten = file.print(content);

    if (bytesWritten > 0)
    {
      DEBUG_PRINTLN(F("File was written"));
      DEBUG_PRINTLN(bytesWritten);
    }
    else
    {
      DEBUG_PRINTLN(F("File write failed"));
    }

    file.close();
    serverWeb.sendHeader(F("Location"), F("/fsbrowser"));
    serverWeb.send(303);
  }
}

void handleLogBuffer()
{
  String result;
  result = logPrint();
  serverWeb.send(200, F("text/html"), result);
}

void handleScanNetwork()
{
  String result = "";
  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    result = " <label for='ssid'>SSID</label>";
    result += "<input class='form-control' id='ssid' type='text' name='WIFISSID' value='{{ssid}}'> <a onclick='scanNetwork();' class='btn btn-primary mb-2'>Scan</a><div id='networks'></div>";
  }
  else
  {

    result = "<select name='WIFISSID' onChange='updateSSID(this.value);'>";
    result += "<OPTION value=''>--Choose SSID--</OPTION>";
    for (int i = 0; i < n; ++i)
    {
      result += "<OPTION value='";
      result += WiFi.SSID(i);
      result += "'>";
      result += WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")";
      result += "</OPTION>";
    }
    result += "</select>";
  }
  serverWeb.send(200, F("text/html"), result);
}
void handleClearConsole()
{
  logClear();

  serverWeb.send(200, F("text/html"), "");
}

void handleGetVersion()
{
  //\01\02\10\10\02\10\02\10\10\03
  char output_sprintf[2];
  uint8_t cmd[10];
  cmd[0] = 0x01;
  cmd[1] = 0x02;
  cmd[2] = 0x10;
  cmd[3] = 0x10;
  cmd[4] = 0x02;
  cmd[5] = 0x10;
  cmd[6] = 0x02;
  cmd[7] = 0x10;
  cmd[8] = 0x10;
  cmd[9] = 0x03;

  Serial2.write(cmd, 10);
  Serial2.flush();

  String buff = "";

  printLogTime();

  logPush('-');
  logPush('>');

  for (int i = 0; i < 10; i++)
  {
    sprintf(output_sprintf, "%02x", cmd[i]);
    logPush(' ');
    logPush(output_sprintf[0]);
    logPush(output_sprintf[1]);
  }
  logPush('\n');
  serverWeb.send(200, F("text/html"), "");
}

void handleZigbeeReset()
{
  serverWeb.send(200, F("text/html"), "");
  printLogMsg("Zigbee RST pin ON");
  digitalWrite(RESET_ZIGBEE, 0);
  delay(100);
  printLogMsg("Zigbee RST pin OFF");
  digitalWrite(RESET_ZIGBEE, 1);
}

void handleZigbeeBSL()
{
  serverWeb.send(200, F("text/html"), "");
  printLogMsg("Zigbee BSL pin ON");
  digitalWrite(FLASH_ZIGBEE, 0);
  delay(100);
  printLogMsg("Zigbee RST pin ON");
  digitalWrite(RESET_ZIGBEE, 0);
  delay(100);
  printLogMsg("Zigbee RST pin OFF");
  digitalWrite(RESET_ZIGBEE, 1);
  delay(2000);
  printLogMsg("Zigbee BSL pin OFF");
  digitalWrite(FLASH_ZIGBEE, 1);
  printLogMsg("Update with cc2538-bsl tool now!");
}

void printLogTime()
{
  String tmpTime;
  timeLog = millis();
  tmpTime = String(timeLog, DEC);
  logPush('[');
  for (int j = 0; j < tmpTime.length(); j++)
  {
    logPush(tmpTime[j]);
  }
  logPush(']');
}

void printLogMsg(String msg)
{
  printLogTime();
  logPush(' ');
  logPush('D');
  logPush('E');
  logPush('V');
  logPush(' ');
  for (int j = 0; j < msg.length(); j++)
  {
    logPush(msg[j]);
  }
  logPush('\n');
}
