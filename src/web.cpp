#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include "WiFi.h"
#include <WebServer.h>
#include "FS.h"
#include <LittleFS.h>
#include "web.h"
#include "config.h"
#include "log.h"
#include "etc.h"
#include <Update.h>
#include "html.h"
#include <HTTPClient.h>

#include "webh/required.css.gz.h"
#include "webh/bootstrap.min.js.gz.h"
#include "webh/functions.js.gz.h"
#include "webh/jquery-min.js.gz.h"
#include "webh/logo.png.gz.h"
#include "webh/masonry.js.gz.h"
#include "webh/favicon.ico.gz.h"

extern struct ConfigSettingsStruct ConfigSettings;

WebServer serverWeb(80);

HTTPClient clientWeb;

void handle_masonry_js();

void webServerHandleClient()
{
  serverWeb.handleClient();
}

void initWebServer()
{
  serverWeb.on("/js/bootstrap.min.js", handle_bootstrap_js);
  serverWeb.on("/js/masonry.min.js", handle_masonry_js);
  serverWeb.on("/js/functions.js", handle_functions_js);
  serverWeb.on("/js/jquery-min.js", handle_jquery_js);
  serverWeb.on("/css/required.css", handle_required_css);
  serverWeb.on("/img/logo.png", handle_logo_png);
  serverWeb.on("/favicon.ico", handle_favicon);
  serverWeb.on("/", handleRoot);
  serverWeb.on("/general", handleGeneral);
  serverWeb.on("/security", handleSecurity);
  serverWeb.on("/wifi", handleRoot);
  serverWeb.on("/ethernet", handleEther);
  serverWeb.on("/serial", handleSerial);

  serverWeb.on("/saveGeneral", HTTP_POST, handleSaveGeneral);
  serverWeb.on("/saveSecurity", HTTP_POST, handleSaveSecurity);
  serverWeb.on("/saveSerial", HTTP_POST, handleSaveSerial);
  serverWeb.on("/saveWifi", HTTP_POST, handleSaveWifi);
  serverWeb.on("/saveEther", HTTP_POST, handleSaveEther);

  //serverWeb.on("/logs-browser", handleLogsBrowser);
  serverWeb.on("/reboot", handleReboot);
  serverWeb.on("/updates", handleUpdate);
  serverWeb.on("/readFile", handleReadfile);
  serverWeb.on("/saveFile", handleSavefile);
  serverWeb.on("/getLogBuffer", handleLogBuffer);
  serverWeb.on("/scanNetwork", handleScanNetwork);
  serverWeb.on("/cmdClearConsole", handleClearConsole);
  serverWeb.on("/cmdZigRST", handleZigbeeRestart);
  serverWeb.on("/cmdZigBSL", handleZigbeeBSL);
  serverWeb.on("/cmdEspReboot", handleReboot);
  serverWeb.on("/cmdAdapterModeUSB", handleAdapterModeUSB);
  serverWeb.on("/cmdAdapterModeLAN", handleAdapterModeLAN);
  serverWeb.on("/cmdLedYellowToggle", handleLedYellowToggle);
  serverWeb.on("/cmdLedBlueToggle", handleLedBlueToggle);      
  serverWeb.on("/switch/firmware_update/toggle", handleZigbeeBSL); //for cc-2538.py ESPHome edition back compatibility | will be disabled on 1.0.0
  serverWeb.on("/about", handleAbout);
  serverWeb.on("/sys-tools", handleSysTools);
  serverWeb.on("/web_update", handleWEBUpdate);
  serverWeb.on("/logged-out", handleLoggedOut);
  serverWeb.onNotFound(handleRoot);//handleNotFound);

  serverWeb.on("/logout", []()
               { serverWeb.send(401); });

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
        if (checkAuth())
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
        }
      });
  serverWeb.begin();
}

void handle_functions_js()
{
  const char *dataType = "text/javascript";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)functions_js_gz, functions_js_gz_len);
}

void handle_bootstrap_js()
{
  const char *dataType = "text/javascript";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)bootstrap_min_js_gz, bootstrap_min_js_gz_len);
}

void handle_masonry_js()
{
  const char *dataType = "text/javascript";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)masonry_js_gz, masonry_js_gz_len);
}

void handle_jquery_js()
{
  const char *dataType = "text/javascript";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)jquery_min_js_gz, jquery_min_js_gz_len);
}

void handle_required_css()
{
  const char *dataType = "text/css";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)required_css_gz, required_css_gz_len);
}

void handle_favicon()
{
  const char *dataType = "img/ico";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)favicon_ico_gz, favicon_ico_gz_len);
}

void handle_logo_png()
{
  const char *dataType = "img/png";
  serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
  serverWeb.send_P(200, dataType, (const char *)logo_png_gz, logo_png_gz_len);
}

void handleLoggedOut()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result.replace("{{logoutLink}}", "");
  result += FPSTR(HTTP_ERROR);
  result += F("</html>");
  result.replace("{{pageName}}", "Logged out");

  serverWeb.send(200, F("text/html"), result);
}

void handleNotFound()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_ERROR);
    result += F("</html>");
    result.replace("{{pageName}}", "Not found - 404");

    serverWeb.send(404, F("text/html"), result);
  }
}

bool checkAuth()
{
  String result;
  result += F("<html>");
  result += FPSTR(HTTP_HEADER);
  result.replace("{{logoutLink}}", "");

  result += FPSTR(HTTP_ERROR);
  result += F("</html>");
  result.replace("{{pageName}}", "Authentication failed");

  const char *www_realm = "Login Required";

  if (ConfigSettings.webAuth && !serverWeb.authenticate(ConfigSettings.webUser, ConfigSettings.webPass))
  {
    serverWeb.requestAuthentication(DIGEST_AUTH, www_realm, result);
    return false;
  }
  else
  {
    return true;
  }
}

void handleAbout()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_ABOUT);
    result += F("</html>");
    result.replace("{{pageName}}", "About");
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    serverWeb.send(200, "text/html", result);
  }
}

void handleGeneral()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_GENERAL);
    result += F("</html>");

    result.replace("{{pageName}}", "General");
    result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);
    result.replace("{{hostname}}", (String)ConfigSettings.hostname);
    DEBUG_PRINTLN(ConfigSettings.usbMode);
    if (ConfigSettings.usbMode)
    {
      result.replace("{{checkedUsbMode}}", "Checked");
    }
    else
    {
      result.replace("{{checkedUsbMode}}", "");
    }
    // DEBUG_PRINTLN(ConfigSettings.disableLedYellow);
    if (ConfigSettings.disableLedYellow)
    {
      result.replace("{{checkedDisableLedYellow}}", "Checked");
    }
    else
    {
      result.replace("{{checkedDisableLedYellow}}", "");
    }
    // DEBUG_PRINTLN(ConfigSettings.disableLedBlue);
    if (ConfigSettings.disableLedBlue)
    {
      result.replace("{{checkedDisableLedBlue}}", "Checked");
    }
    else
    {
      result.replace("{{checkedDisableLedBlue}}", "");
    }
    serverWeb.send(200, "text/html", result);
  }
}
void handleSecurity()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_SECURITY);
    result += F("</html>");

    result.replace("{{pageName}}", "Security");

    if (ConfigSettings.disableWeb)
    {
      result.replace("{{disableWeb}}", "checked");
    }
    else
    {
      result.replace("{{disableWeb}}", "");
    }

    if (ConfigSettings.webAuth)
    {
      result.replace("{{webAuth}}", "checked");
    }
    else
    {
      result.replace("{{webAuth}}", "");
    }
    result.replace("{{webUser}}", (String)ConfigSettings.webUser);
    result.replace("{{webPass}}", (String)ConfigSettings.webPass);

    serverWeb.send(200, "text/html", result);
  }
}

void handleSaveSucces(String msg)
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<div class='col py-3'>");
    result += F("<h2>{{pageName}}</h2>");
    result += F("<div id='main' class='col-sm-12'>");
    result += F("<div id='main' class='col-sm-6'>");
    result += F("<form method='GET' action='reboot' id='upload_form'>");
    result += F("<label>Save ");
    result += msg;
    result += F(" OK !</label><br><br><br>");
    result += F("<button type='submit' class='btn btn-success btn-lg mb-2'>Click to reboot</button>");
    result += F("</form></div></div></div></div></div>");
    result += F("</html>");
    result.replace("{{pageName}}", "Saved");

    serverWeb.send(200, "text/html", result);
  }
}

void handleWifi()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_WIFI);
    result += F("</html>");

    result.replace("{{pageName}}", "Config WiFi");

    DEBUG_PRINTLN(ConfigSettings.enableWiFi);
    if (ConfigSettings.enableWiFi)
    {

      result.replace("{{checkedWiFi}}", "Checked");
    }
    else
    {
      result.replace("{{checkedWiFi}}", "");
    }
    DEBUG_PRINTLN(ConfigSettings.disableEmerg);
    if (ConfigSettings.disableEmerg)
    {
      result.replace("{{checkedDisEmerg}}", "Checked");
    }
    else
    {
      result.replace("{{checkedDisEmerg}}", "");
    }
    result.replace("{{ssid}}", String(ConfigSettings.ssid));
    result.replace("{{passWifi}}", String(ConfigSettings.password));
    if (ConfigSettings.dhcpWiFi)
    {
      result.replace("{{dchp}}", "Checked");
    }
    else
    {
      result.replace("{{dchp}}", "");
    }
    result.replace("{{ip}}", ConfigSettings.ipAddressWiFi);
    result.replace("{{mask}}", ConfigSettings.ipMaskWiFi);
    result.replace("{{gw}}", ConfigSettings.ipGWWiFi);

    serverWeb.send(200, "text/html", result);
  }
}

void handleSerial()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_SERIAL);
    result += F("</html>");

    result.replace("{{pageName}}", "Config Serial");

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
}

void handleEther()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += FPSTR(HTTP_ETHERNET);
    result += F("</html>");

    result.replace("{{pageName}}", "Config Ethernet");

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

    if (ConfigSettings.disablePingCtrl)
    {
      result.replace("{{disablePingCtrl}}", "Checked");
    }
    else
    {
      result.replace("{{disablePingCtrl}}", "");
    }

    serverWeb.send(200, "text/html", result);
  }
}

void handleRoot()
{
    if (checkAuth()) {
    String result;
        result += F("<html>");
        result += FPSTR(HTTP_HEADER);
        if (ConfigSettings.webAuth) {
            result.replace("{{logoutLink}}", LOGOUT_LINK);
        }
        else {
            result.replace("{{logoutLink}}", "");
        }
        result += FPSTR(HTTP_ROOT);
        result += F("</html>");

        result.replace("{{pageName}}", "Status");
        result.replace("{{VERSION}}", VERSION);

    String readableTime;
        getReadableTime(readableTime, ConfigSettings.socketTime);
        if (ConfigSettings.connectedClients > 0) {
            if (ConfigSettings.connectedClients > 1) {
                result.replace("{{connectedSocketStatus}}", "Yes, " + String(ConfigSettings.connectedClients) + "connection");        
                result.replace("{{connectedSocket}}", readableTime);                           
            }
            else {
                result.replace("{{connectedSocketStatus}}", "Yes, " + String(ConfigSettings.connectedClients) + " connections");      
                result.replace("{{connectedSocket}}", readableTime);                                     
            }
        }
        else {
            result.replace("{{connectedSocketStatus}}", "No");    
            result.replace("{{connectedSocket}}", "Not connected");         
        }

        if (ConfigSettings.usbMode) {
            result.replace("{{operationalMode}}", "Zigbee-to-USB");
        }
        else {
            result.replace("{{operationalMode}}", "Zigbee-to-Ethernet");
        }
        if (ConfigSettings.enableWiFi) {
            result.replace("{{wifiEnabled}}", "Yes");
            result.replace("{{wifiConnected}}", "Not connected");            
        }
        else {
            result.replace("{{wifiEnabled}}", "No");
            result.replace("{{wifiConnected}}", "Disabled");
        }
        if (ConfigSettings.disableEmerg) {
            result.replace("{{wifiModeAP}}", "No");
            result.replace("{{wifiModeAPStatus}}", "Disabled");
        }
        else {
            result.replace("{{wifiModeAP}}", "Yes");
            result.replace("{{wifiModeAPStatus}}", "Not started");            
        }
        if (ConfigSettings.connectedEther) {
            result.replace("{{connectedEther}}", "Yes");
        }
        else {
            result.replace("{{connectedEther}}", "No");
        }

// ETHERNET TAB
        if (ConfigSettings.connectedEther) {
            result.replace("{{ethConnection}}", "Connected");
            result.replace("{{ethMac}}", ETH.macAddress());
            result.replace("{{ethSpd}}", String(ETH.linkSpeed()) + String(" Mbps"));
            if (ConfigSettings.dhcp) {
                result.replace("{{ethDhcp}}", "On");
                result.replace("{{ethIp}}", ETH.localIP().toString());
                result.replace("{{etchMask}}", ETH.subnetMask().toString());
                result.replace("{{ethGate}}", ETH.gatewayIP().toString());
            }
            else {
                result.replace("{{ethDhcp}}", "Off");
                result.replace("{{ethIp}}", ConfigSettings.ipAddress);
                result.replace("{{etchMask}}", ConfigSettings.ipMask);
                result.replace("{{ethGate}}", ConfigSettings.ipGW);
            }
        }
        else {
            result.replace("{{ethConnection}}", "Not connected");
            result.replace("{{ethMac}}", "Not connected");
            result.replace("{{ethSpd}}", "Not connected");
            if (ConfigSettings.dhcp) {
                result.replace("{{ethDhcp}}", "On");
                result.replace("{{ethIp}}", "Not connected");
                result.replace("{{etchMask}}", "Not connected");
                result.replace("{{ethGate}}", "Not connected");
            }
            else {
                result.replace("{{ethDhcp}}", "Off");
                result.replace("{{ethIp}}", ConfigSettings.ipAddress);
                result.replace("{{etchMask}}", ConfigSettings.ipMask);
                result.replace("{{ethGate}}", ConfigSettings.ipGW);
            }
        }
        getReadableTime(readableTime, 0);
        result.replace("{{uptime}}", readableTime);

    float CPUtemp = getCPUtemp();
        result.replace("{{deviceTemp}}", String(CPUtemp));
        result.replace("{{hwRev}}", BOARD_NAME);
        result.replace("{{espModel}}", String(ESP.getChipModel()));
        result.replace("{{espCores}}", String(ESP.getChipCores()));
        result.replace("{{espFreq}}", String(ESP.getCpuFreqMHz()));
        result.replace("{{espHeapFree}}", String(ESP.getFreeHeap() / 1024));
        result.replace("{{espHeapSize}}", String(ESP.getHeapSize() / 1024));

    esp_chip_info_t chip_info;
        esp_chip_info(& chip_info);

        if (chip_info.features & CHIP_FEATURE_EMB_FLASH) {
            result.replace("{{espFlashType}}", "embedded");
        }
        else {
            result.replace("{{espFlashType}}", "external");
        }

        result.replace("{{espFlashSize}}", String(ESP.getFlashChipSize() / (1024 * 1024)));

    String wifiState = "<strong>Enabled : </strong>";
    result.replace("{{wifiMode}}", "Off");
    result.replace("{{wifiSsid}}", "Off");
    result.replace("{{wifiMac}}", "Off");
    result.replace("{{wifiIp}}", "Off");
    result.replace("{{wifiSubnet}}", "Off");
    result.replace("{{wifiGate}}", "Off");  
    result.replace("{{wifiRssi}}", "Off");
    result.replace("{{wifiDhcp}}", "Off");   
                    if (ConfigSettings.dhcpWiFi) {
                        result.replace("{{wifiDhcp}}", "On");
                    }
                    else {
                        result.replace("{{wifiIp}}", ConfigSettings.ipAddressWiFi);
                        result.replace("{{wifiSubnet}}", ConfigSettings.ipMaskWiFi);
                        result.replace("{{wifiGate}}", ConfigSettings.ipGWWiFi);  
                        result.replace("{{wifiDhcp}}", "Off");  
//                        wifiState = wifiState + "STATIC<br><strong>IP : </strong>" + ConfigSettings.ipAddressWiFi;
//                        wifiState = wifiState + "<br><strong>Mask : </strong>" + ConfigSettings.ipMaskWiFi;
//                        wifiState = wifiState + "<br><strong>GW : </strong>" + ConfigSettings.ipGWWiFi;
                    }


        if (WiFi.getMode() == WIFI_MODE_STA || WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {
            result.replace("{{wifiMode}}", "Client");             
//            wifiState += "<img src='/img/ok.png'>";
            if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {
                wifiState += "<strong> Emergency mode</strong>";
            }
//            wifiState += "<br><strong>MAC : </strong>" + WiFi.softAPmacAddress();
//            wifiState += "<br><strong>Mode : </strong> ";
            if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {
                result.replace("{{wifiIp}}", WiFi.localIP().toString());  
        String AP_NameString;
                getDeviceID(AP_NameString);
                result.replace("{{wifiSsid}}", AP_NameString + ", no password");
                result.replace("{{wifiIp}}", "192.168.10.1 (SLZB-06 web interface)");
                result.replace("{{wifiSubnet}}", "255.255.255.0 (Access point)");
                result.replace("{{wifiGate}}", "192.168.10.1 (this device)");  
                result.replace("{{wifiDhcp}}", "On (Access point)");    
                result.replace("{{wifiModeAPStatus}}", "AP started");                                  
//                wifiState += "AP <br><strong>SSID : </strong>" + AP_NameString;
//                wifiState += "<br>No password";
//                wifiState += "<br><strong>IP : </strong>192.168.4.1";
            }
            else {
        int rssi = WiFi.RSSI();
        String rssiWifi = String(rssi) + String(" dBm");
                wifiState = wifiState + "STA <br><strong>SSID : </strong>" + ConfigSettings.ssid;
                result.replace("{{wifiConnected}}", "Yes, to " + String(ConfigSettings.ssid));                      
                wifiState += "<br><strong>Connected : </strong>";
                if (rssi != 0) {
                    result.replace("{{wifiDhcp}}", "On (client)");                      
                    wifiState += "<img src='/img/ok.png'>";
                    wifiState += "<br><strong>RSSI : </strong>" + rssiWifi;
                    wifiState += "<br><strong>Mode : </strong>";
                    if (ConfigSettings.dhcpWiFi) {
                        result.replace("{{wifiIp}}", WiFi.localIP().toString());
                        result.replace("{{wifiSubnet}}", WiFi.subnetMask().toString());
                        result.replace("{{wifiGate}}", WiFi.gatewayIP().toString());                                                
//                        wifiState += "DHCP<br><strong>IP : </strong>" + WiFi.localIP().toString();
//                        wifiState += "<br><strong>Mask : </strong>" + WiFi.subnetMask().toString();
//                        wifiState += "<br><strong>GW : </strong>" + WiFi.gatewayIP().toString();
                    }
                    else {
                        result.replace("{{wifiIp}}", ConfigSettings.ipAddressWiFi);
                        result.replace("{{wifiSubnet}}", ConfigSettings.ipMaskWiFi);
                        result.replace("{{wifiGate}}", ConfigSettings.ipGWWiFi);  
//                        wifiState = wifiState + "STATIC<br><strong>IP : </strong>" + ConfigSettings.ipAddressWiFi;
//                        wifiState = wifiState + "<br><strong>Mask : </strong>" + ConfigSettings.ipMaskWiFi;
//                        wifiState = wifiState + "<br><strong>GW : </strong>" + ConfigSettings.ipGWWiFi;
                    }
                }
                else {
                    wifiState += "<img src='/img/nok.png'>";
                }
            }
        }
        else {
            wifiState += "<img src='/img/nok.png'>";
        }
        result.replace("{{stateWifi}}", wifiState);

        serverWeb.send(200, "text/html", result);
    }
}

void handleSaveGeneral()
{
  if (checkAuth())
  {
    String StringConfig;
    String refreshLogs;
    String hostname;
    String usbMode;
    String disableLedYellow;
    String disableLedBlue;

    if (serverWeb.arg("refreshLogs").toDouble() < 1000)
    {
      refreshLogs = "1000";
    }
    else
    {
      refreshLogs = serverWeb.arg("refreshLogs");
    }

    if (serverWeb.arg("hostname") != "")
    {
      hostname = serverWeb.arg("hostname");
    }
    else
    {
      hostname = "SLZB-06";
    }
    //DEBUG_PRINTLN(hostname);
    if (serverWeb.arg("usbMode") == "on")
    {
      usbMode = "1";
    }
    else
    {
      usbMode = "0";
    }
    if (serverWeb.arg("disableLedYellow") == "on")
    {
      disableLedYellow = "1";
    }
    else
    {
      disableLedYellow = "0";
      ConfigSettings.disableLeds = false;
    }
    if (serverWeb.arg("disableLedBlue") == "on")
    {
      disableLedBlue = "1";
    }
    else
    {
      disableLedBlue = "0";
      ConfigSettings.disableLeds = false;
    }
    const char *path = "/config/configGeneral.json";

    StringConfig = "{\"refreshLogs\":" + refreshLogs + ",\"hostname\":\"" + hostname + "\",\"disableLeds\": " + String(ConfigSettings.disableLeds) + ",\"usbMode\":\"" + usbMode + "\",\"disableLedYellow\":\"" + disableLedYellow + "\",\"disableLedBlue\":\"" + disableLedBlue + "\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LittleFS.open(path, FILE_WRITE);
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
}

void handleSaveSecurity()
{
  if (checkAuth())
  {
    String StringConfig;
    String disableWeb;
    String webAuth;
    String webUser;
    String webPass;

    if (serverWeb.arg("disableWeb") == "on")
    {
      disableWeb = "1";
    }
    else
    {
      disableWeb = "0";
    }

    if (serverWeb.arg("webAuth") == "on")
    {
      webAuth = "1";
    }
    else
    {
      webAuth = "0";
    }

    if (serverWeb.arg("webUser") != "")
    {
      webUser = serverWeb.arg("webUser");
    }
    else
    {
      webUser = "admin";
    }

    if (serverWeb.arg("webPass") != "")
    {
      webPass = serverWeb.arg("webPass");
    }
    else
    {
      webPass = "admin";
    }

    //DEBUG_PRINTLN(hostname);
    const char *path = "/config/configSecurity.json";

    StringConfig = "{\"disableWeb\":" + disableWeb + ",\"webAuth\":" + webAuth + ",\"webUser\":\"" + webUser + "\",\"webPass\":\"" + webPass + "\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LittleFS.open(path, FILE_WRITE);
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
}

void handleSaveWifi()
{
  if (checkAuth())
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
    String disableEmerg;
    if (serverWeb.arg("disableEmerg") == "on")
    {
      disableEmerg = "1";
      saveEmergencyWifi(0);
    }
    else
    {
      disableEmerg = "0";
    }
    const char *path = "/config/configWifi.json";

    StringConfig = "{\"enableWiFi\":" + enableWiFi + ",\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"dhcpWiFi\":" + dhcpWiFi + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\",\"disableEmerg\":\"" + disableEmerg + "\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LittleFS.open(path, FILE_WRITE);
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
}

void handleSaveSerial()
{
  if (checkAuth())
  {
    String StringConfig;
    String serialSpeed = serverWeb.arg("baud");
    String socketPort = serverWeb.arg("port");
    const char *path = "/config/configSerial.json";

    StringConfig = "{\"baud\":" + serialSpeed + ", \"port\":" + socketPort + "}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LittleFS.open(path, FILE_WRITE);
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
}

void handleSaveEther()
{
  if (checkAuth())
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

    String disablePingCtrl;
    if (serverWeb.arg("disablePingCtrl") == "on")
    {
      disablePingCtrl = "1";
    }
    else
    {
      disablePingCtrl = "0";
    }

    const char *path = "/config/configEther.json";

    StringConfig = "{\"dhcp\":" + dhcp + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\",\"disablePingCtrl\":\"" + disablePingCtrl + "\"}";
    DEBUG_PRINTLN(StringConfig);
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, StringConfig);

    File configFile = LittleFS.open(path, FILE_WRITE);
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
}

void handleReboot()
{
  if (checkAuth())
  {
    String result;

    result += F("<html>");
    result += F("<meta http-equiv='refresh' content='1; URL=/'>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<div class='col py-3'>");
    result += F("<h2>{{pageName}}</h2>");
    result += F("</div></div></div>");    
    result = result + F("</body></html>");
    result.replace("{{pageName}}", "Rebooted");

    serverWeb.send(200, F("text/html"), result);

    ESP.restart();
  }
}

void handleUpdate()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    result += F("<h2>{{pageName}}</h2>");
    result += F("<div class='btn-group-vertical'>");
    result += F("<a href='/setchipid' class='btn btn-primary mb-2'>setChipId</button>");
    result += F("<a href='/setmodeprod' class='btn btn-primary mb-2'>setModeProd</button>");
    result += F("</div>");

    result = result + F("</body></html>");
    result.replace("{{pageName}}", "Update Zigbee");

    serverWeb.send(200, F("text/html"), result);
  }
}

void handleSysTools()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    // result += F("<div class='col py-3'>");
    // result += F("<h2>{{pageName}}</h2>");
    result += FPSTR(HTTP_SYSTOOLS);
    //result += F("</div></div>");
    result = result + F("</body></html>");
    result.replace("{{pageName}}", "System and Tools");
    serverWeb.sendHeader("Connection", "close");
    
    String fileList = "";
    File root = LittleFS.open("/config");
    File file = root.openNextFile();
    while (file)
    {
      String tmp = file.name();
      //DEBUG_PRINTLN(tmp);
      //tmp = tmp.substring(8);
      fileList += F("<tr>");
      fileList += F("<td><svg xmlns='http://www.w3.org/2000/svg' width='20' height='20' fill='currentColor' class='bi bi-filetype-json' viewBox='0 0 16 16'><path fill-rule='evenodd' d='M14 4.5V11h-1V4.5h-2A1.5 1.5 0 0 1 9.5 3V1H4a1 1 0 0 0-1 1v9H2V2a2 2 0 0 1 2-2h5.5L14 4.5ZM4.151 15.29a1.176 1.176 0 0 1-.111-.449h.764a.578.578 0 0 0 .255.384c.07.049.154.087.25.114.095.028.201.041.319.041.164 0 .301-.023.413-.07a.559.559 0 0 0 .255-.193.507.507 0 0 0 .084-.29.387.387 0 0 0-.152-.326c-.101-.08-.256-.144-.463-.193l-.618-.143a1.72 1.72 0 0 1-.539-.214 1.001 1.001 0 0 1-.352-.367 1.068 1.068 0 0 1-.123-.524c0-.244.064-.457.19-.639.128-.181.304-.322.528-.422.225-.1.484-.149.777-.149.304 0 .564.05.779.152.217.102.384.239.5.41.12.17.186.359.2.566h-.75a.56.56 0 0 0-.12-.258.624.624 0 0 0-.246-.181.923.923 0 0 0-.37-.068c-.216 0-.387.05-.512.152a.472.472 0 0 0-.185.384c0 .121.048.22.144.3a.97.97 0 0 0 .404.175l.621.143c.217.05.406.12.566.211a1 1 0 0 1 .375.358c.09.148.135.335.135.56 0 .247-.063.466-.188.656a1.216 1.216 0 0 1-.539.439c-.234.105-.52.158-.858.158-.254 0-.476-.03-.665-.09a1.404 1.404 0 0 1-.478-.252 1.13 1.13 0 0 1-.29-.375Zm-3.104-.033a1.32 1.32 0 0 1-.082-.466h.764a.576.576 0 0 0 .074.27.499.499 0 0 0 .454.246c.19 0 .33-.055.422-.164.091-.11.137-.265.137-.466v-2.745h.791v2.725c0 .44-.119.774-.357 1.005-.237.23-.565.345-.985.345a1.59 1.59 0 0 1-.568-.094 1.145 1.145 0 0 1-.407-.266 1.14 1.14 0 0 1-.243-.39Zm9.091-1.585v.522c0 .256-.039.47-.117.641a.862.862 0 0 1-.322.387.877.877 0 0 1-.47.126.883.883 0 0 1-.47-.126.87.87 0 0 1-.32-.387 1.55 1.55 0 0 1-.117-.641v-.522c0-.258.039-.471.117-.641a.87.87 0 0 1 .32-.387.868.868 0 0 1 .47-.129c.177 0 .333.043.47.129a.862.862 0 0 1 .322.387c.078.17.117.383.117.641Zm.803.519v-.513c0-.377-.069-.701-.205-.973a1.46 1.46 0 0 0-.59-.63c-.253-.146-.559-.22-.916-.22-.356 0-.662.074-.92.22a1.441 1.441 0 0 0-.589.628c-.137.271-.205.596-.205.975v.513c0 .375.068.699.205.973.137.271.333.48.589.626.258.145.564.217.92.217.357 0 .663-.072.917-.217.256-.146.452-.355.589-.626.136-.274.205-.598.205-.973Zm1.29-.935v2.675h-.746v-3.999h.662l1.752 2.66h.032v-2.66h.75v4h-.656l-1.761-2.676h-.032Z'/></svg><a href='#config_file' onClick=\"readfile('");
      fileList += tmp;
      fileList += F("');\">");
      fileList += tmp;
      fileList += F("</a></td>");
      fileList += F("<td>");
      fileList += file.size();
      fileList += F("B</td>");
      fileList += F("<tr>");
      file = root.openNextFile();
    }
    result.replace("{{fileList}}", fileList);
    result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);

    serverWeb.send(200, "text/html", result);
  }
}

// void handleLogsBrowser()
// {
//   if (checkAuth())
//   {
//     String result;
//     result += F("<html>");
//     result += FPSTR(HTTP_HEADER);
//     result += FPSTR(LOGS_BROWSER);
//     if (ConfigSettings.webAuth)
//     {
//       result.replace("{{logoutLink}}", LOGOUT_LINK);
//     }
//     else
//     {
//       result.replace("{{logoutLink}}", "");
//     }
// //     result += F("<div class='col py-3'>");
// //     result += F("<h2>{{pageName}}</h2>");
// //     result += F("<div id='main' class='col-sm-12'>");
// //     result += F("<div id='help_btns' class='col-md-11'>");
//     result.replace("{{pageName}}", "Logs and Browser");

//     String fileList = "";
//     File root = LittleFS.open("/config");
//     File file = root.openNextFile();
//     while (file)
//     {
//       String tmp = file.name();
//       //DEBUG_PRINTLN(tmp);
//       //tmp = tmp.substring(8);
//       fileList += F("<tr>");
//       fileList += F("<td><svg xmlns='http://www.w3.org/2000/svg' width='20' height='20' fill='currentColor' class='bi bi-filetype-json' viewBox='0 0 16 16'><path fill-rule='evenodd' d='M14 4.5V11h-1V4.5h-2A1.5 1.5 0 0 1 9.5 3V1H4a1 1 0 0 0-1 1v9H2V2a2 2 0 0 1 2-2h5.5L14 4.5ZM4.151 15.29a1.176 1.176 0 0 1-.111-.449h.764a.578.578 0 0 0 .255.384c.07.049.154.087.25.114.095.028.201.041.319.041.164 0 .301-.023.413-.07a.559.559 0 0 0 .255-.193.507.507 0 0 0 .084-.29.387.387 0 0 0-.152-.326c-.101-.08-.256-.144-.463-.193l-.618-.143a1.72 1.72 0 0 1-.539-.214 1.001 1.001 0 0 1-.352-.367 1.068 1.068 0 0 1-.123-.524c0-.244.064-.457.19-.639.128-.181.304-.322.528-.422.225-.1.484-.149.777-.149.304 0 .564.05.779.152.217.102.384.239.5.41.12.17.186.359.2.566h-.75a.56.56 0 0 0-.12-.258.624.624 0 0 0-.246-.181.923.923 0 0 0-.37-.068c-.216 0-.387.05-.512.152a.472.472 0 0 0-.185.384c0 .121.048.22.144.3a.97.97 0 0 0 .404.175l.621.143c.217.05.406.12.566.211a1 1 0 0 1 .375.358c.09.148.135.335.135.56 0 .247-.063.466-.188.656a1.216 1.216 0 0 1-.539.439c-.234.105-.52.158-.858.158-.254 0-.476-.03-.665-.09a1.404 1.404 0 0 1-.478-.252 1.13 1.13 0 0 1-.29-.375Zm-3.104-.033a1.32 1.32 0 0 1-.082-.466h.764a.576.576 0 0 0 .074.27.499.499 0 0 0 .454.246c.19 0 .33-.055.422-.164.091-.11.137-.265.137-.466v-2.745h.791v2.725c0 .44-.119.774-.357 1.005-.237.23-.565.345-.985.345a1.59 1.59 0 0 1-.568-.094 1.145 1.145 0 0 1-.407-.266 1.14 1.14 0 0 1-.243-.39Zm9.091-1.585v.522c0 .256-.039.47-.117.641a.862.862 0 0 1-.322.387.877.877 0 0 1-.47.126.883.883 0 0 1-.47-.126.87.87 0 0 1-.32-.387 1.55 1.55 0 0 1-.117-.641v-.522c0-.258.039-.471.117-.641a.87.87 0 0 1 .32-.387.868.868 0 0 1 .47-.129c.177 0 .333.043.47.129a.862.862 0 0 1 .322.387c.078.17.117.383.117.641Zm.803.519v-.513c0-.377-.069-.701-.205-.973a1.46 1.46 0 0 0-.59-.63c-.253-.146-.559-.22-.916-.22-.356 0-.662.074-.92.22a1.441 1.441 0 0 0-.589.628c-.137.271-.205.596-.205.975v.513c0 .375.068.699.205.973.137.271.333.48.589.626.258.145.564.217.92.217.357 0 .663-.072.917-.217.256-.146.452-.355.589-.626.136-.274.205-.598.205-.973Zm1.29-.935v2.675h-.746v-3.999h.662l1.752 2.66h.032v-2.66h.75v4h-.656l-1.761-2.676h-.032Z'/></svg><a href='#' onClick=\"readfile('");
//       fileList += tmp;
//       fileList += F("');\">");
//       fileList += tmp;
//       fileList += F("</a></td>");
//       fileList += F("<td>");
//       fileList += file.size();
//       fileList += F("B</td>");
//       fileList += F("<tr>");
//       file = root.openNextFile();
//     }
//     result.replace("{{fileList}}", fileList);
// //     result += F("</div>");
// //     result += F("<div id='main' class='col-md-9'>");
// //     result += F("<div class='app-main-content'>");
// //     result += F("<form method='POST' action='saveFile'>");
// //     result += F("<div class='form-group'>");
// //     result += F("<div><label for='file'>File : <span id='title'></span></label>");
// //     result += F("<input type='hidden' name='filename' id='filename' value=''></div>");
// //     result += F("<textarea class='form-control' id='file' name='file' rows='10'>");
// //     result += F("</textarea>");
// //     result += F("</div>");
// //     result += F("<button type='submit' class='btn btn-primary mb-2'>Save</button>");
// //     result += F("</form>");
// //     result += F("</div>");
// //     result += F("</div>");
// //     result += F("</div>");

// //     result += F("<br>");
// //     result += F("<div id='main' class='col-sm-12'>");
// //     result += F("<div id='help_btns' class='col-sm-8'>");
// //     result += F("<button type='button' onclick='cmd(\"ClearConsole\");document.getElementById(\"console\").value=\"\";' class='btn btn-secondary'>Clear Console</button> ");
// // //#ifdef DEBUG
// // //    result += F("<button type='button' onclick='cmd(\"GetVersion\");' class='btn btn-success'>Get Version</button> ");
// // //    result += F("<button type='button' onclick='cmd(\"ZigRestart\");' class='btn btn-danger'>Zig Restart</button> ");
// // //#endif
// //     result += F("</div></div>");
// //     result += F("<div id='main' class='col-sm-8'>");
// //     result += F("<div class='col-md-12'>Raw data :</div>");
// //     result += F("<textarea class='col-md-12' id='console' rows='8' ></textarea>");
// //     result += F("</div>");
// //     result += F("</body>");

// //     result += F("<script language='javascript'>");
// //     result += F("logRefresh({{refreshLogs}});");
// //     result += F("</script>");
// //     result += F("</div></div></div>");
//     result += F("</body></html>");
//     result.replace("{{refreshLogs}}", (String)ConfigSettings.refreshLogs);

//     serverWeb.send(200, F("text/html"), result);
//   }
// }

void handleReadfile()
{
  if (checkAuth())
  {
    String result;
    String filename = "/config/" + serverWeb.arg(0);
    File file = LittleFS.open(filename, "r");

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
}

void handleSavefile()
{
  if (checkAuth())
  {
    if (serverWeb.method() != HTTP_POST)
    {
      serverWeb.send(405, F("text/plain"), F("Method Not Allowed"));
    }
    else
    {
      String filename = "/config/" + serverWeb.arg(0);
      String content = serverWeb.arg(1);
      File file = LittleFS.open(filename, "w");
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
      serverWeb.sendHeader(F("Location"), F("/sys-tools"));
      serverWeb.send(303);
    }
  }
}

void handleLogBuffer()
{
  if (checkAuth())
  {
    String result;
    result = logPrint();

    serverWeb.send(200, F("text/html"), result);
  }
}

void handleScanNetwork()
{
  if (checkAuth())
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
}
void handleClearConsole()
{
  if (checkAuth())
  {
    logClear();

    serverWeb.send(200, F("text/html"), "");
  }
}

/*
void handleGetVersion()
{
  cmdGetZigVersion();
}

void handleZigRestart()
{
  cmdZigRestart();
}
*/

void handleZigbeeRestart()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    zigbeeRestart();
  }
}

void handleZigbeeBSL()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    zigbeeEnableBSL();
  }
}

void handleAdapterModeUSB()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    adapterModeUSB();
  }
}

void handleAdapterModeLAN()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    adapterModeLAN();
  }
}

void handleLedYellowToggle()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    ledYellowToggle();
  }
}

void handleLedBlueToggle()
{
  if (checkAuth())
  {
    serverWeb.send(200, F("text/html"), "");

    ledBlueToggle();
  }
}

void printLogTime()
{
  String tmpTime;
  unsigned long timeLog = millis();
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
  logPush('|');
  logPush(' ');
  for (int j = 0; j < msg.length(); j++)
  {
    logPush(msg[j]);
  }
  logPush('\n');
}

void handleWEBUpdate()
{
  if (checkAuth())
  {
    String result;
    result += F("<html>");
    result += FPSTR(HTTP_HEADER);
    if (ConfigSettings.webAuth)
    {
      result.replace("{{logoutLink}}", LOGOUT_LINK);
    }
    else
    {
      result.replace("{{logoutLink}}", "");
    }
    
    result += F("<h2>{{pageName}}</h2>");
    //result += FPSTR(HTTP_SYSTOOLS);
    result = result + F("</body></html>");
    result.replace("{{pageName}}", "Update ESP32");
    serverWeb.sendHeader("Connection", "close");

    serverWeb.send(200, "text/html", result);
    checkUpdateFirmware();
  }
}

int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware

void progressFunc(unsigned int progress,unsigned int total) {
  Serial.printf("Progress: %u of %u\r", progress, total);
};


void checkUpdateFirmware()
{
  clientWeb.begin(UPD_FILE);
  clientWeb.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); 
  // Get file, just to check if each reachable
  int resp = clientWeb.GET();
  Serial.print("Response: ");
  Serial.println(resp);
  // If file is reachable, start downloading
  if(resp == HTTP_CODE_OK) 
  {   
      // get length of document (is -1 when Server sends no Content-Length header)
      totalLength = clientWeb.getSize();
      // transfer to local variable
      int len = totalLength;
      // this is required to start firmware update process
      Update.begin(UPDATE_SIZE_UNKNOWN);
      Update.onProgress(progressFunc);
      DEBUG_PRINT("FW Size: ");
      
      DEBUG_PRINTLN(totalLength);
      // create buffer for read
      uint8_t buff[128] = { 0 };
      // get tcp stream
      WiFiClient * stream = clientWeb.getStreamPtr();
      // read all data from server
      DEBUG_PRINTLN("Updating firmware...");
      while(clientWeb.connected() && (len > 0 || len == -1)) {
          // get available data size
          size_t size = stream->available();
          if(size) {
            // read up to 128 byte
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            // pass to function
            runUpdateFirmware(buff, c);
            if(len > 0) {
                len -= c;
            }
          }
          //DEBUG_PRINT("Bytes left to flash ");
          //DEBUG_PRINTLN(len);
           //delay(1);
      }
  }
  else
  {
    Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in UPD_FILE.");
  }
  clientWeb.end();
}

void runUpdateFirmware(uint8_t *data, size_t len)
{
  Update.write(data, len);
  currentLength += len;
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if(currentLength != totalLength) return;
  Update.end(true);
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
  // Restart ESP32 to see changes 
  ESP.restart();
}