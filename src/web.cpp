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
#include <Update.h>

extern struct ConfigSettingsStruct ConfigSettings;
extern unsigned long timeLog;

String style =
    "<style>#file-input,input{width:100%;height:44px;border-radius:4px;margin:10px auto;font-size:15px}"
    "input{background:#f1f1f1;border:0;padding:0 15px}body{font-size:14px;color:#777}"
    "#file-input{padding:0;border:1px solid #ddd;line-height:44px;text-align:left;display:block;cursor:pointer}"
    "#bar,#prgbar{background-color:#f1f1f1;border-radius:10px}#bar{background-color:#3498db;width:0%;height:10px}"
    "form{background:#fff;max-width:408px;margin:75px auto;padding:30px;border-radius:5px;text-align:center}"
    "</style>";

/* Server Index Page */
String serverIndex =
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
    "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
    "<input type='file' name='update' id='file' onchange='sub(this)' style=display:none>"
    "<label id='file-input' for='file'>   Choose file...</label>"
    "<input type='submit' class='btn btn-primary mb-2' value='Update'>"
    "<br><br>"
    "<div id='prg'></div>"
    "<br><div id='prgbar'><div id='bar'></div></div><br></form>"
    "<script>"
    "function sub(obj){"
    "var fileName = obj.value.split('\\\\');"
    "document.getElementById('file-input').innerHTML = '   '+ fileName[fileName.length-1];"
    "};"
    "$('form').submit(function(e){"
    "e.preventDefault();"
    "var form = $('#upload_form')[0];"
    "var data = new FormData(form);"
    "$.ajax({"
    "url: '/update',"
    "type: 'POST',"
    "data: data,"
    "contentType: false,"
    "processData:false,"
    "xhr: function() {"
    "var xhr = new window.XMLHttpRequest();"
    "xhr.upload.addEventListener('progress', function(evt) {"
    "if (evt.lengthComputable) {"
    "var per = evt.loaded / evt.total;"
    "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
    "$('#bar').css('width',Math.round(per*100) + '%');"
    "}"
    "}, false);"
    "return xhr;"
    "},"
    "success:function(d, s) {"
    "console.log('success!') "
    "},"
    "error: function (a, b, c) {"
    "}"
    "});"
    "});"
    "</script>" +
    style;

WebServer serverWeb(80);

void webServerHandleClient()
{
  serverWeb.handleClient();
}

const char HTTP_HEADER[] PROGMEM =
    "<head>"
    "<script type='text/javascript' src='web/js/jquery-min.js'></script>"
    "<script type='text/javascript' src='web/js/bootstrap.min.js'></script>"
    "<script type='text/javascript' src='web/js/functions.js'></script>"
    "<link href='web/css/bootstrap.min.css' rel='stylesheet' type='text/css' />"
    "<link href='web/css/style.css' rel='stylesheet' type='text/css' />"
    " </head>"
    "<body>"
    "<nav class='navbar navbar-expand-lg navbar-light bg-info rounded'><a class='navbar-brand' href='/'><img src='web/img/logo.png'/> <strong>Config </strong>" VERSION
    "</a>"
    "<button class='navbar-toggler' type='button' data-toggle='collapse' data-target='#navbarNavDropdown' aria-controls='navbarNavDropdown' aria-expanded='false' aria-label='Toggle navigation'>"
    "<span class='navbar-toggler-icon'></span>"
    "</button>"
    "<div id='navbarNavDropdown' class='collapse navbar-collapse justify-content-md-center'>"
    "<ul class='navbar-nav'>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='/'>Status</a>"
    "</li>"
    "<li class='nav-item dropdown'>"
    "<a class='nav-link dropdown-toggle' href='#' id='navbarDropdown' role='button' data-toggle='dropdown' aria-haspopup='true' aria-expanded='false'>Config</a>"
    "<div class='dropdown-menu' aria-labelledby='navbarDropdown'>"
    "<a class='dropdown-item' href='/general'>General</a>"
    "<a class='dropdown-item' href='/serial'>Serial</a>"
    "<a class='dropdown-item' href='/ethernet'>Ethernet</a>"
    "<a class='dropdown-item' href='/wifi'>WiFi</a>"
    "</div>"
    "</li>"
    "<li class='nav-item dropdown'>"
    "<a class='nav-link dropdown-toggle' href='#' id='navbarDropdown' role='button' data-toggle='dropdown' aria-haspopup='true' aria-expanded='false'>Tools</a>"
    "<div class='dropdown-menu' aria-labelledby='navbarDropdown'>"
    "<a class='dropdown-item' href='/logs'>Console</a>"
    "<a class='dropdown-item' href='/fsbrowser'>FSbrowser</a>"
    "<a class='dropdown-item' href='/esp_update'>Update ESP32</a>"
    "<a class='dropdown-item' href='/updates'>Update Zigbee</a>"
    "<a class='dropdown-item' href='/reboot'>Reboot</a>"
    "</div>"
    "</li>"
    "<li class='nav-item'>"
    "<a class='nav-link' href='/help'>Help</a>"
    "</li>"
    "</ul></div>"
    "</nav>";

const char HTTP_WIFI[] PROGMEM =
    "<h1>Config WiFi</h1>"
    "<div class='row justify-content-md-center' >"
    "<div class='col-sm-6'><form method='POST' action='saveWifi'>"
    "<div class='form-check'>"

    "<input class='form-check-input' id='wifiEnable' type='checkbox' name='wifiEnable' {{checkedWiFi}}>"
    "<label class='form-check-label' for='wifiEnable'>Enable</label>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ssid'>SSID</label>"
    "<input class='form-control' id='ssid' type='text' name='WIFISSID' value='{{ssid}}'> <a onclick='scanNetwork();' class='btn btn-primary mb-2'>Scan</a><div id='networks'></div>"
    "</div>"
    "<div class='form-group'>"
    "<label for='pass'>Password</label>"
    "<input class='form-control' id='pass' type='password' name='WIFIpassword' value=''>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ip'>IP</label>"
    "<input class='form-control' id='ip' type='text' name='ipAddress' value='{{ip}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='mask'>Mask</label>"
    "<input class='form-control' id='mask' type='text' name='ipMask' value='{{mask}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='gateway'>Gateway</label>"
    "<input type='text' class='form-control' id='gateway' name='ipGW' value='{{gw}}'>"
    "</div>"
    "<button type='submit' class='btn btn-primary mb-2' name='save'>Save</button>"
    "</form>";

const char HTTP_SERIAL[] PROGMEM =
    "<h1>Config Serial</h1>"
    "<div class='row justify-content-md-center' >"
    "<div class='col-sm-6'><form method='POST' action='saveSerial'>"

    "<div class='form-group'>"
    "<label for='baud'>Serial Speed</label>"
    "<select class='form-control' id='baud' name='baud'>"
    "<option value='9600' {{selected9600}}>9600 bauds</option>"
    "<option value='19200' {{selected19200}}>19200 bauds</option>"
    "<option value='38400' {{selected38400}}>38400 bauds</option>"
    "<option value='57600' {{selected57600}}>57600 bauds</option>"
    "<option value='115200' {{selected115200}}>115200 bauds</option>"
    "</select>"
    "<label for='port'>Socket Port</label>"
    "<input class='form-control' id='port' type='number' name='port' min='100' max='65000' value='{{socketPort}}'>"
    "</div>"
    "<br><br>"
    "<button type='submit' class='btn btn-primary mb-2'name='save'>Save</button>"
    "</form>";

const char HTTP_HELP[] PROGMEM =
    "<h1>Help !</h1>"
    "<div class='row justify-content-md-center' >"
    "<h3>Shop & description</h3>"
    "You can go to this url :</br>"
    "<a href=\"https://zigate.fr/boutique\" target='_blank'>Shop </a></br>"
    "<a href=\"https://zigate.fr/documentation/descriptif-de-la-zigate-ethernet/\" target='_blank'>Description</a></br>"
    "<h3>Firmware Source & Issues</h3>"
    "Please go here :</br>"
    "<a href=\"https://github.com/fairecasoimeme/ZiGate-Ethernet\" target='_blank'>Sources</a>"
    "/<div>";

const char HTTP_ETHERNET[] PROGMEM =
    "<h1>Config Ethernet</h1>"
    "<div class='row justify-content-md-center' >"
    "<div class='col-sm-6'><form method='POST' action='saveEther'>"
    "<div class='form-check'>"

    "<input class='form-check-input' id='dhcp' type='checkbox' name='dhcp' {{modeEther}}>"
    "<label class='form-check-label' for='dhcp'>DHCP</label>"
    "</div>"
    "<div class='form-group'>"
    "<label for='ip'>IP</label>"
    "<input class='form-control' id='ip' type='text' name='ipAddress' value='{{ipEther}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='mask'>Mask</label>"
    "<input class='form-control' id='mask' type='text' name='ipMask' value='{{maskEther}}'>"
    "</div>"
    "<div class='form-group'>"
    "<label for='gateway'>Gateway</label>"
    "<input type='text' class='form-control' id='gateway' name='ipGW' value='{{GWEther}}'>"
    "</div>"
    "<button type='submit' class='btn btn-primary mb-2'name='save'>Save</button>"
    "</form>";

const char HTTP_GENERAL[] PROGMEM =
    "<h1>General</h1>"
    "<div class='row justify-content-md-center' >"
    "<div class='col-sm-6'><form method='POST' action='saveGeneral'>"
    "<div class='form-group'>"
    "<label for='hostname'>Hostname</label>"
    "<input class='form-control' id='hostname' type='text' name='hostname' value='{{hostname}}'>"
    "</div>"
    "<div class='form-check'>"
    "<input class='form-check-input' id='disableWeb' type='checkbox' name='disableWeb' {{disableWeb}}>"
    "<label class='form-check-label' for='disableWeb'>Disable web server when ZiGate is connected</label>"
    "<br>"
    "<input class='form-check-input' id='enableHeartBeat' type='checkbox' name='enableHeartBeat' {{enableHeartBeat}}>"
    "<label class='form-check-label' for='enableHeartBeat'>Enable HeartBeat (send ping to TCP when no trafic)</label>"
    "<br>"
    "<label for='refreshLogs'>Refresh console log</label>"
    "<input class='form-control' id='refreshLogs' type='text' name='refreshLogs' value='{{refreshLogs}}'>"
    "<br>"
    "</div>"
    "<button type='submit' class='btn btn-primary mb-2' name='save'>Save</button>"
    "</form></div>"
    "</div>";

const char HTTP_ROOT[] PROGMEM =
    "<h1>Status</h1>"
    "<div class='row justify-content-md-center'>"
    "<div class='col-sm-6'>"
    "<div class='card'>"
    "<div class='card-header'>Ethernet</div>"
    "<div class='card-body'>"
    "<div id='ethConfig'>"
    "<strong>Connected : </strong>{{connectedEther}}"
    "<br><strong>Mode : </strong>{{modeEther}}"
    "<br><strong>IP : </strong>{{ipEther}}"
    "<br><strong>Mask : </strong>{{maskEther}}"
    "<br><strong>GW : </strong>{{GWEther}}"
    "</div>"
    "</div>"
    "</div>"
    "</div>"
    "</div>"
    "<div class='row justify-content-md-center'>"
    "<div class='col-sm-6'>"
    "<div class='card'>"
    "<div class='card-header'>Wifi</div>"
    "<div class='card-body'>"
    "<div id='wifiConfig'>"
    "<strong>Enable : </strong>{{enableWifi}}"
    "<br><strong>SSID : </strong>{{ssidWifi}}"
    "<br><strong>IP : </strong>{{ipWifi}}"
    "<br><strong>Mask : </strong>{{maskWifi}}"
    "<br><strong>GW : </strong>{{GWWifi}}"
    "</div>"
    "</div>"
    "</div>"
    "</div>"
    "</div>"

    ;

void initWebServer()
{
  serverWeb.serveStatic("/web/js/jquery-min.js", LITTLEFS, "/web/js/jquery-min.js");
  serverWeb.serveStatic("/web/js/functions.js", LITTLEFS, "/web/js/functions.js");
  serverWeb.serveStatic("/web/js/bootstrap.min.js", LITTLEFS, "/web/js/bootstrap.min.js");
  serverWeb.serveStatic("/web/js/bootstrap.min.js.map", LITTLEFS, "/web/js/bootstrap.min.js.map");
  serverWeb.serveStatic("/web/css/bootstrap.min.css", LITTLEFS, "/web/css/bootstrap.min.css");
  serverWeb.serveStatic("/web/css/style.css", LITTLEFS, "/web/css/style.css");
  serverWeb.serveStatic("/web/img/logo.png", LITTLEFS, "/web/img/logo.png");
  serverWeb.serveStatic("/web/img/wait.gif", LITTLEFS, "/web/img/wait.gif");
  serverWeb.serveStatic("/web/img/nok.png", LITTLEFS, "/web/img/nok.png");
  serverWeb.serveStatic("/web/img/ok.png", LITTLEFS, "/web/img/ok.png");
  serverWeb.serveStatic("/web/img/", LITTLEFS, "/web/img/");
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
  result += F("<h1>Saved</h1>");
  result += F("<div class='row justify-content-md-center'>");
  result += F("<div class='col-sm-6'>");
  result += F("<div class='form-group'><label>Save ");
  result += msg;
  result += F(" OK !</label></div>");
  result += F("<br><form method='GET' action='reboot'>");
  result += F("<input type='submit' class='btn btn-warning mb-2' name='reboot' value='Reboot'></form>");
  result += F("</div></div>");
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

  if (ConfigSettings.enableWiFi)
  {
    result.replace("{{enableWifi}}", "<img src='/web/img/ok.png'>");
  }
  else
  {
    result.replace("{{enableWifi}}", "<img src='/web/img/nok.png'>");
  }
  result.replace("{{ssidWifi}}", String(ConfigSettings.ssid));
  result.replace("{{ipWifi}}", ConfigSettings.ipAddressWiFi);
  result.replace("{{maskWifi}}", ConfigSettings.ipMaskWiFi);
  result.replace("{{GWWifi}}", ConfigSettings.ipGWWiFi);

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
  String ipAddress = serverWeb.arg("ipAddress");
  String ipMask = serverWeb.arg("ipMask");
  String ipGW = serverWeb.arg("ipGW");

  const char *path = "/config/configWifi.json";

  StringConfig = "{\"enableWiFi\":" + enableWiFi + ",\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
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
  result += F("<h1>Console</h1>");
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
  result += FPSTR(HTTP_HEADER);
  result += F("<h1>Reboot ...</h1>");
  result = result + F("</body></html>");
  serverWeb.sendHeader(F("Location"), F("/"));
  serverWeb.send(303);

  ESP.restart();
}

void handleUpdate()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h1>Update Zigbee</h1>");
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
  result += F("<h1>Update ESP32</h1>");
  result += serverIndex;
  result = result + F("</body></html>");
  serverWeb.sendHeader("Connection", "close");
  serverWeb.send(200, "text/html", result);
}

void handleFSbrowser()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result += F("<h1>FSBrowser</h1>");
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
