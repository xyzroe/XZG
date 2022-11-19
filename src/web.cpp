#include "web.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <Update.h>
#include <WebServer.h>

#include "FS.h"
#include "WiFi.h"
#include "config.h"
#include "etc.h"
#include "html.h"
#include "log.h"
#include "webh/PAGE_ABOUT.html.gz.h"
#include "webh/PAGE_ETHERNET.html.gz.h"
#include "webh/PAGE_GENERAL.html.gz.h"
#include "webh/PAGE_LOADER.html.gz.h"
#include "webh/PAGE_ROOT.html.gz.h"
#include "webh/PAGE_SECURITY.html.gz.h"
#include "webh/PAGE_SERIAL.html.gz.h"
#include "webh/PAGE_SYSTOOLS.html.gz.h"
#include "webh/PAGE_WIFI.html.gz.h"
#include "webh/bootstrap.min.js.gz.h"
#include "webh/favicon.ico.gz.h"
#include "webh/functions.js.gz.h"
#include "webh/jquery-min.js.gz.h"
#include "webh/logo.png.gz.h"
#include "webh/masonry.js.gz.h"
#include "webh/required.css.gz.h"
#include "webh/PAGE_LOGOUT.html.gz.h"

extern struct ConfigSettingsStruct ConfigSettings;
extern const char *coordMode;
const char *contTypeTextHtml = "text/html";
const char *contTypeTextJs = "text/javascript";
const char *contTypeTextCss = "text/css";
const char *checked = "true";
const char *respHeaderName = "respValuesArr";
const char *contTypeJson = "application/json";
const char *contTypeText = "text/plain";

enum API_PAGE_t : uint8_t { API_PAGE_ROOT,
                                API_PAGE_GENERAL,
                                API_PAGE_ETHERNET,
                                API_PAGE_WIFI,
                                API_PAGE_SERIAL,
                                API_PAGE_SECURITY,
                                API_PAGE_SYSTOOLS,
                                API_PAGE_ABOUT };

WebServer serverWeb(80);

HTTPClient clientWeb;

void handle_masonry_js();

void webServerHandleClient() {
    serverWeb.handleClient();
}

void initWebServer() {
    serverWeb.on("/js/bootstrap.min.js", []() { sendGzip(contTypeTextJs, bootstrap_min_js_gz, bootstrap_min_js_gz_len); });
    serverWeb.on("/js/masonry.min.js", []() { sendGzip(contTypeTextJs, masonry_js_gz, masonry_js_gz_len); });
    serverWeb.on("/js/functions.js", []() { sendGzip(contTypeTextJs, functions_js_gz, functions_js_gz_len); });
    serverWeb.on("/js/jquery-min.js", []() { sendGzip(contTypeTextJs, jquery_min_js_gz, jquery_min_js_gz_len); });
    serverWeb.on("/css/required.css", []() { sendGzip(contTypeTextJs, required_css_gz, required_css_gz_len); });
    serverWeb.on("/img/logo.png", []() { sendGzip("img/ico", logo_png_gz, logo_png_gz_len); });
    serverWeb.on("/favicon.ico", []() { sendGzip("img/png", favicon_ico_gz, favicon_ico_gz_len); });

    serverWeb.onNotFound([]() { serverWeb.send(HTTP_CODE_OK, contTypeText, F("URL NOT FOUND")); });  // handleNotFound);
    serverWeb.on("/", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });
    serverWeb.on("/general", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });
    serverWeb.on("/security", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });
    serverWeb.on("/wifi", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });
    serverWeb.on("/ethernet", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });
    serverWeb.on("/serial", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });
    serverWeb.on("/about", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });
    serverWeb.on("/sys-tools", []() { sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len); });

    // serverWeb.on("/saveGeneral", HTTP_POST, handleSaveGeneral);
    // serverWeb.on("/saveSecurity", HTTP_POST, handleSaveSecurity);
    // serverWeb.on("/saveSerial", HTTP_POST, handleSaveSerial);
    // serverWeb.on("/saveWifi", HTTP_POST, handleSaveWifi);
    // serverWeb.on("/saveEther", HTTP_POST, handleSaveEther);
    serverWeb.on("/saveParams", HTTP_POST, handleSaveParams);

    // serverWeb.on("/logs-browser", handleLogsBrowser);
    serverWeb.on("/reboot", handleReboot);
    // serverWeb.on("/updates", handleUpdate);
    //serverWeb.on("/readFile", handleReadfile);
    serverWeb.on("/saveFile", handleSavefile);
    serverWeb.on("/getLogBuffer", handleLogBuffer);
    // serverWeb.on("/wifiScan", handleScanNetwork);
    // serverWeb.on("/wifiScanStatus", handleScanNetworkDone);
    serverWeb.on("/cmdClearConsole", handleClearConsole);
    serverWeb.on("/cmdZigRST", handleZigbeeRestart);
    serverWeb.on("/cmdZigBSL", handleZigbeeBSL);
    serverWeb.on("/cmdEspReboot", handleReboot);
    serverWeb.on("/cmdAdapterModeUSB", handleAdapterModeUSB);
    serverWeb.on("/cmdAdapterModeLAN", handleAdapterModeLAN);
    serverWeb.on("/cmdLedYellowToggle", handleLedYellowToggle);
    serverWeb.on("/cmdLedBlueToggle", handleLedBlueToggle);
    serverWeb.on("/switch/firmware_update/toggle", handleZigbeeBSL);  // for cc-2538.py ESPHome edition back compatibility | will be disabled on 1.0.0
    // serverWeb.on("/web_update", handleWEBUpdate);
    //serverWeb.on("/logged-out", handleLoggedOut);
    serverWeb.on("/api", handleApi);
    serverWeb.on("/logout", []() { 
        serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
        serverWeb.send_P(401, contTypeTextHtml, (const char *)PAGE_LOGOUT_html_gz, PAGE_LOGOUT_html_gz_len);
    });

    /*handling uploading firmware file */
    serverWeb.on(
        "/update", HTTP_POST, []() {
        serverWeb.sendHeader("Connection", "close");
        serverWeb.send(HTTP_CODE_OK, contTypeText, (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart(); },
        []() {
            if (checkAuth()) {
                HTTPUpload &upload = serverWeb.upload();
                if (upload.status == UPLOAD_FILE_START) {
                    Serial.printf("Update: %s\n", upload.filename.c_str());
                    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {  // start with max available size
                        Update.printError(Serial);
                    }
                } else if (upload.status == UPLOAD_FILE_WRITE) {
                    /* flashing firmware to ESP*/
                    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                        Update.printError(Serial);
                    }
                } else if (upload.status == UPLOAD_FILE_END) {
                    if (Update.end(true)) {  // true to set the size to the current progress
                        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                        serverWeb.send(HTTP_CODE_OK, contTypeText, (Update.hasError()) ? "FAIL" : "OK");
                    } else {
                        Update.printError(Serial);
                    }
                }
            }
        });
    serverWeb.begin();
    DEBUG_PRINTLN(F("webserver setup done"));
}

void sendGzip(const char *contentType, const uint8_t content[], uint16_t contentLen) {
    serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
    serverWeb.send_P(HTTP_CODE_OK, contentType, (const char *)content, contentLen);
}

void handleApi() {  // http://192.168.0.116/api?action=0&page=0
    enum API_ACTION_t : uint8_t { API_GET_PAGE,
                                  API_GET_PARAM,
                                  API_STARTWIFISCAN,
                                  API_WIFISCANSTATUS,
                                  API_GET_FILELIST,
                                  API_GET_FILE };
    const char *action = "action";
    const char *page = "page";
    const char *Authentication = "Authentication";
    const char* param = "param";

    if(ConfigSettings.webAuth){
        if(!checkAuth()){//Authentication
            serverWeb.sendHeader(Authentication, "fail");
            serverWeb.send(401, contTypeText, F("wrong login or password"));
            return;
        }else{
            serverWeb.sendHeader(Authentication, "ok");
        }
    }
    if (serverWeb.argName(0) != action) {
        DEBUG_PRINTLN(F("[handleApi] wrong arg 'action'"));
        DEBUG_PRINTLN(serverWeb.argName(0));
        serverWeb.send(500, contTypeText, F("wrong args"));
    } else {
        const uint8_t action = serverWeb.arg(action).toInt();
        DEBUG_PRINTLN(F("[handleApi] arg 0 is:"));
        DEBUG_PRINTLN(action);
        switch (action) {
            case API_GET_FILE:{
                String result = "wrong params";
                if(serverWeb.hasArg("filename")){
                    String filename = "/config/" + serverWeb.arg("filename");
                    File file = LittleFS.open(filename, "r");
                    if (!file) return;
                    result = "";              
                    while (file.available()) {
                        result += (char)file.read();
                    }
                    file.close();    
                }
                serverWeb.send(HTTP_CODE_OK, contTypeText, result);
            }
            break;
            case API_GET_PARAM:{
                    String resp = "wrong params";
                    if(serverWeb.hasArg(param)){
                        if (serverWeb.arg(param) == "refreshLogs"){
                            resp = (String)ConfigSettings.refreshLogs;
                        }
                    }
                    serverWeb.send(HTTP_CODE_OK, contTypeText, resp);
                }
                break;
            case API_STARTWIFISCAN:
                    if (WiFi.getMode() == WIFI_OFF) {  // enable wifi for scan
                        WiFi.mode(WIFI_STA);
                        WiFi.scanNetworks(true);
                    } else if (WiFi.getMode() == WIFI_AP) {  // enable sta for scan
                        WiFi.mode(WIFI_AP_STA);
                    }
                    serverWeb.send(HTTP_CODE_OK, contTypeTextHtml, "ok");
                break;
            case API_WIFISCANSTATUS: {
                static uint8_t timeout = 0;
                DynamicJsonDocument doc(1024);
                String result = "";
                int16_t scanRes = WiFi.scanComplete();
                doc["scanDone"] = false;
                if (scanRes == -2) {
                    WiFi.scanNetworks(true);
                } else if (scanRes > 0) {
                    doc["scanDone"] = true;
                    JsonArray wifi = doc.createNestedArray("wifi");
                    for (int i = 0; i < scanRes; ++i) {
                        JsonObject wifi_0 = wifi.createNestedObject();
                        wifi_0["ssid"] = WiFi.SSID(i);
                        wifi_0["rssi"] = WiFi.RSSI(i);
                        wifi_0["channel"] = WiFi.channel(i);
                        wifi_0["secure"] = WiFi.encryptionType(i);
                    }
                    WiFi.scanDelete();
                    if (ConfigSettings.coordinator_mode == COORDINATOR_MODE_LAN) WiFi.mode(WIFI_OFF);
                }
                if (timeout < 10) {
                    timeout++;
                } else {
                    doc["scanDone"] = true;
                    WiFi.scanDelete();
                    timeout = 0;
                    if (ConfigSettings.coordinator_mode == COORDINATOR_MODE_LAN) WiFi.mode(WIFI_OFF);
                }

                serializeJson(doc, result);
                serverWeb.send(HTTP_CODE_OK, contTypeJson, result);
                break;
            }
            case API_GET_PAGE:
                if (!serverWeb.arg(page).length() > 0) {
                    DEBUG_PRINTLN(F("[handleApi] wrong arg 'page'"));
                    DEBUG_PRINTLN(serverWeb.argName(1));
                    serverWeb.send(500, contTypeText, F("wrong args"));
                    return;
                }
                switch (serverWeb.arg(page).toInt()) {
                    case API_PAGE_ROOT:
                        handleRoot();
                        sendGzip(contTypeTextHtml, PAGE_ROOT_html_gz, PAGE_ROOT_html_gz_len);
                        break;
                    case API_PAGE_GENERAL:
                        handleGeneral();
                        sendGzip(contTypeTextHtml, PAGE_GENERAL_html_gz, PAGE_GENERAL_html_gz_len);
                        break;
                    case API_PAGE_ETHERNET:
                        handleEther();
                        sendGzip(contTypeTextHtml, PAGE_ETHERNET_html_gz, PAGE_ETHERNET_html_gz_len);
                        break;
                    case API_PAGE_WIFI:
                        handleWifi();
                        sendGzip(contTypeTextHtml, PAGE_WIFI_html_gz, PAGE_WIFI_html_gz_len);
                        break;
                    case API_PAGE_SERIAL:
                        handleSerial();
                        sendGzip(contTypeTextHtml, PAGE_SERIAL_html_gz, PAGE_SERIAL_html_gz_len);
                        break;
                    case API_PAGE_SECURITY:
                        handleSecurity();
                        sendGzip(contTypeTextHtml, PAGE_SECURITY_html_gz, PAGE_SECURITY_html_gz_len);
                        break;
                    case API_PAGE_SYSTOOLS:
                        handleSysTools();
                        sendGzip(contTypeTextHtml, PAGE_SYSTOOLS_html_gz, PAGE_SYSTOOLS_html_gz_len);
                        break;
                    case API_PAGE_ABOUT:
                        handleAbout();
                        sendGzip(contTypeTextHtml, PAGE_ABOUT_html_gz, PAGE_ABOUT_html_gz_len);
                        break;

                    default:
                        break;
                }
                break;
            case API_GET_FILELIST:{
                String fileList = "";
                DynamicJsonDocument doc(512);
                JsonArray files = doc.createNestedArray("files");
                File root = LittleFS.open("/config");
                File file = root.openNextFile();
                while (file) {
                    JsonObject jsonfile = files.createNestedObject();
                    jsonfile["filename"] = String(file.name());
                    jsonfile["size"] = file.size();
                    file = root.openNextFile();
                }
                serializeJson(doc, fileList);
                serverWeb.send(HTTP_CODE_OK, contTypeJson, fileList);
                break;
            }

            default:
                DEBUG_PRINTLN(F("[handleApi] switch (action) err"));
                break;
        }
    }
}

void handleSaveParams(){
    String result;
    DynamicJsonDocument doc(512);
    const char* pageId = "pageId";
    const char* on = "on";
    File configFile;
    const uint8_t one = 1;
    const uint8_t zero = 0;
    if (serverWeb.hasArg(pageId)){
       switch (serverWeb.arg(pageId).toInt()){
            case API_PAGE_GENERAL:{
                const char *path = "/config/configGeneral.json";
                configFile = LittleFS.open(path, FILE_READ);
                deserializeJson(doc, configFile);
                configFile.close();

                if (serverWeb.hasArg(coordMode)) {
                    const uint8_t mode = serverWeb.arg(coordMode).toInt();
                    if(mode <= 2 && mode >= zero){
                        ConfigSettings.coordinator_mode = static_cast<COORDINATOR_MODE_t>(mode);
                        doc[coordMode] = ConfigSettings.coordinator_mode;
                    }           
                }
                const char* disableLedYellow = "disableLedYellow";
                if (serverWeb.arg(disableLedYellow) == on) {
                    doc[disableLedYellow] = one;
                } else {
                    doc[disableLedYellow] = zero;
                }
                const char* disableLedBlue = "disableLedBlue";
                if (serverWeb.arg(disableLedBlue) == on) {
                    doc[disableLedBlue] = one;
                } else {
                    doc[disableLedBlue] = zero;
                }
                configFile = LittleFS.open(path, FILE_WRITE);
                serializeJson(doc, configFile);
                configFile.close();
            }
            break;
            case API_PAGE_ETHERNET:{
                const char *path = "/config/configEther.json";
                configFile = LittleFS.open(path, FILE_READ);
                deserializeJson(doc, configFile);
                configFile.close();
                doc["ip"] = serverWeb.arg("ipAddress");
                doc["mask"] = serverWeb.arg("ipMask");
                doc["gw"] = serverWeb.arg("ipGW");
                const char* dhcp = "dhcp";
                if (serverWeb.arg(dhcp) == "on") {
                    doc[dhcp] = one;
                } else {
                    doc[dhcp] = zero;
                }
                const char* disablePingCtrl = "disablePingCtrl";
                if (serverWeb.arg(disablePingCtrl) == "on") {
                    doc[disablePingCtrl] = one;
                } else {
                    doc[disablePingCtrl] = zero;
                }
                configFile = LittleFS.open(path, FILE_WRITE);
                serializeJson(doc, configFile);
                configFile.close();
            }
            break;
            case API_PAGE_WIFI:{
                const char *path = "/config/configWifi.json";
                configFile = LittleFS.open(path, FILE_READ);
                deserializeJson(doc, configFile);
                configFile.close();
                doc["ssid"] = serverWeb.arg("WIFISSID");
                doc["pass"] = serverWeb.arg("WIFIpassword");
                const char* dhcpWiFi = "dhcpWiFi";
                if (serverWeb.arg(dhcpWiFi) == on) {
                    doc[dhcpWiFi] = 1;
                } else {
                    doc[dhcpWiFi] = 0;
                }
                doc["ip"] = serverWeb.arg("ipAddress");
                doc["mask"] = serverWeb.arg("ipMask");
                doc["gw"] = serverWeb.arg("ipGW");
                configFile = LittleFS.open(path, FILE_WRITE);
                serializeJson(doc, configFile);
                configFile.close();
            }
            break;
            case API_PAGE_SERIAL:{
                const char *path = "/config/configSerial.json";
                configFile = LittleFS.open(path, FILE_READ);
                deserializeJson(doc, configFile);
                configFile.close();
                const char* baud = "baud";
                if (serverWeb.hasArg(baud)){
                    doc[baud] = serverWeb.arg(baud);
                }else{
                    doc[baud] = "115200";
                }
                const char* port = "port";
                if (serverWeb.hasArg(baud)){
                    doc[port] = serverWeb.arg(port);
                }else{
                    doc[port] = "6638";
                }
                configFile = LittleFS.open(path, FILE_WRITE);
                serializeJson(doc, configFile);
                configFile.close();
            }
            break;
            case API_PAGE_SECURITY:{
                const char *path = "/config/configSecurity.json";
                configFile = LittleFS.open(path, FILE_READ);
                deserializeJson(doc, configFile);
                configFile.close();
                const char* disableWeb = "disableWeb";
                if (serverWeb.arg(disableWeb) == "on") {
                    doc[disableWeb] = 1;
                } else {
                    doc[disableWeb] = 0;
                }
                const char* webAuth = "webAuth";
                if (serverWeb.arg(webAuth) == "on") {
                    doc[webAuth] = 1;
                } else {
                    doc[webAuth] = 0;
                }
                const char* webUser = "webUser";
                if (serverWeb.arg(webUser) != "") {
                    doc[webUser] = serverWeb.arg(webUser);
                } else {
                    doc[webUser] = "admin";
                }
                doc["webPass"] = serverWeb.arg("webPass");
                configFile = LittleFS.open(path, FILE_WRITE);
                serializeJson(doc, configFile);
                configFile.close();
            }
            break;
            case API_PAGE_SYSTOOLS:{
                const char *path = "/config/configGeneral.json";
                const char *refreshLogs = "refreshLogs";
                const char *hostname = "hostname";
                configFile = LittleFS.open(path, FILE_READ);
                deserializeJson(doc, configFile);
                configFile.close();
                if (serverWeb.hasArg(refreshLogs)){
                    ConfigSettings.refreshLogs = serverWeb.arg(refreshLogs).toInt();
                    doc[refreshLogs] = ConfigSettings.refreshLogs;
                }
                if (serverWeb.hasArg(hostname)){
                    doc[hostname] = serverWeb.arg(hostname);
                    strlcpy(ConfigSettings.hostname, serverWeb.arg(hostname).c_str(), sizeof(ConfigSettings.hostname));
                }
                configFile = LittleFS.open(path, FILE_WRITE);
                serializeJson(doc, configFile);
                configFile.close();
            }
            break;
       
            default:
            break;
       }
       serverWeb.send(HTTP_CODE_OK, contTypeText, "ok");
    }else{
        serverWeb.send(500, contTypeText, "bad args");
    }
}

// void handleLoggedOut() {
//     String result;
//     result += F("<html>");
//     // result += FPSTR(HTTP_HEADER);
//     result.replace("{{logoutLink}}", "");
//     result += FPSTR(HTTP_ERROR);
//     result += F("</html>");
//     result.replace("{{pageName}}", "Logged out");

//     serverWeb.send(200, F("text/html"), result);
// }

// void handleNotFound() {
//     if (checkAuth()) {
//         String result;
//         result += F("<html>");
//         // result += FPSTR(HTTP_HEADER);
//         if (ConfigSettings.webAuth) {
//             result.replace("{{logoutLink}}", LOGOUT_LINK);
//         } else {
//             result.replace("{{logoutLink}}", "");
//         }
//         result += FPSTR(HTTP_ERROR);
//         result += F("</html>");
//         result.replace("{{pageName}}", "Not found - 404");

//         serverWeb.send(404, F("text/html"), result);
//     }
// }

bool checkAuth() {
    //String result;
    //result += F("<html>");
    // result += FPSTR(HTTP_HEADER);
    //result.replace("{{logoutLink}}", "");

    //result += FPSTR(HTTP_ERROR);
    //result += F("</html>");
    //result.replace("{{pageName}}", "Authentication failed");
    if(!ConfigSettings.webAuth) return true;
    const char *www_realm = "Login Required";
    if (!serverWeb.authenticate(ConfigSettings.webUser, ConfigSettings.webPass)) {
        serverWeb.requestAuthentication(DIGEST_AUTH, www_realm, "Authentication failed");
        return false;
    } else {
        return true;
    }
}

void handleAbout() {
        String result;
        DynamicJsonDocument doc(512);
        //result += F("<html>");
        // result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     doc["logoutLink"] = LOGOUT_LINK;
        // } else {
        //     doc["logoutLink"] = "";
        // }
        // result += FPSTR(HTTP_ABOUT);
        //result += F("</html>");
        doc["pageName"] = "About";
        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}

void handleGeneral() {
        DynamicJsonDocument doc(1024);
        String result;
        // result += F("<html>");
        //  result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     doc["logoutLink"] = LOGOUT_LINK;
        // } else {
        //     doc["logoutLink"] = "";
        // }
        // result += FPSTR(HTTP_GENERAL);
        // result += F("</html>");

        doc["pageName"] = "General";
        // DEBUG_PRINTLN(ConfigSettings.usbMode);
        switch (ConfigSettings.coordinator_mode) {
            case COORDINATOR_MODE_USB:
                doc["checkedUsbMode"] = checked;
                break;
            case COORDINATOR_MODE_WIFI:
                doc["checkedWifiMode"] = checked;
                break;
            case COORDINATOR_MODE_LAN:
                doc["checkedLanMode"] = checked;
                break;

            default:
                break;
        }
        // DEBUG_PRINTLN(ConfigSettings.disableLedYellow);
        if (ConfigSettings.disableLedYellow) {
            doc["checkedDisableLedYellow"] = checked;
        }
        // DEBUG_PRINTLN(ConfigSettings.disableLedBlue);
        if (ConfigSettings.disableLedBlue) {
            doc["checkedDisableLedBlue"] = checked;
        }
        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}
void handleSecurity() {
        String result;
        DynamicJsonDocument doc(1024);
        // result += F("<html>");
        //  result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     doc["logoutLink"] = LOGOUT_LINK;
        // } else {
        //     doc["logoutLink"] = "";
        // }
        // result += FPSTR(HTTP_SECURITY);
        // result += F("</html>");

        doc["pageName"] = "Security";

        if (ConfigSettings.disableWeb) {
            doc["disableWeb"] = checked;
        }

        if (ConfigSettings.webAuth) {
            doc["webAuth"] = checked;
        }
        doc["webUser"] = (String)ConfigSettings.webUser;
        doc["webPass"] = (String)ConfigSettings.webPass;

        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}

// void handleSaveSucces(String msg) {//todo remove
//         String result;
//         result += F("<html>");
//         // result += FPSTR(HTTP_HEADER);
//         if (ConfigSettings.webAuth) {
//             result.replace("{{logoutLink}}", LOGOUT_LINK);
//         } else {
//             result.replace("{{logoutLink}}", "");
//         }
//         result += F("<div class='col py-3'>");
//         result += F("<h2>{{pageName}}</h2>");
//         result += F("<div id='main' class='col-sm-12'>");
//         result += F("<div id='main' class='col-sm-6'>");
//         result += F("<form method='GET' action='reboot' id='upload_form'>");
//         result += F("<label>Save ");
//         result += msg;
//         result += F(" OK !</label><br><br><br>");
//         result += F("<button type='submit' class='btn btn-success btn-lg mb-2'>Click to reboot</button>");
//         result += F("</form></div></div></div></div></div>");
//         result += F("</html>");
//         result.replace("{{pageName}}", "Saved");

//         serverWeb.send(200, "text/html", result);
// }

void handleWifi() {
    // if (checkAuth()) {
    //     String result;
    //     result += F("<html>");
    //     result += FPSTR(HTTP_HEADER);
    //     if (ConfigSettings.webAuth) {
    //         result.replace("{{logoutLink}}", LOGOUT_LINK);
    //     } else {
    //         result.replace("{{logoutLink}}", "");
    //     }
    //     result += FPSTR(HTTP_WIFI);
    //     result += F("</html>");

    //     result.replace("{{pageName}}", "Config WiFi");

    //     DEBUG_PRINTLN(ConfigSettings.enableWiFi);
    //     if (ConfigSettings.enableWiFi) {
    //         result.replace("{{checkedWiFi}}", "Checked");
    //     } else {
    //         result.replace("{{checkedWiFi}}", "");
    //     }
    //     DEBUG_PRINTLN(ConfigSettings.disableEmerg);
    //     if (ConfigSettings.disableEmerg) {
    //         result.replace("{{checkedDisEmerg}}", "Checked");
    //     } else {
    //         result.replace("{{checkedDisEmerg}}", "");
    //     }
    //     result.replace("{{ssid}}", String(ConfigSettings.ssid));
    //     result.replace("{{passWifi}}", String(ConfigSettings.password));
    //     if (ConfigSettings.dhcpWiFi) {
    //         result.replace("{{dchp}}", "Checked");
    //     } else {
    //         result.replace("{{dchp}}", "");
    //     }
    //     result.replace("{{ip}}", ConfigSettings.ipAddressWiFi);
    //     result.replace("{{mask}}", ConfigSettings.ipMaskWiFi);
    //     result.replace("{{gw}}", ConfigSettings.ipGWWiFi);

    //     serverWeb.send(200, "text/html", result);
    // }
        String result;
        DynamicJsonDocument doc(1024);
        // result += F("<html>");
        // result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     doc["logoutLink"] = LOGOUT_LINK;
        // } else {
        //     doc["logoutLink"] = "";
        // }
        // result += FPSTR(HTTP_WIFI);
        // result += F("</html>");

        doc["pageName"] = "Config WiFi";

        // DEBUG_PRINTLN(ConfigSettings.enableWiFi);
        // if (ConfigSettings.enableWiFi) {
        //     result.replace("{{checkedWiFi}}", "Checked");
        // } else {
        //     result.replace("{{checkedWiFi}}", "");
        // }
        // DEBUG_PRINTLN(ConfigSettings.disableEmerg);
        // if (ConfigSettings.disableEmerg) {
        //     result.replace("{{checkedDisEmerg}}", "Checked");
        // } else {
        //     result.replace("{{checkedDisEmerg}}", "");
        // }
        doc["ssid"] = String(ConfigSettings.ssid);
        doc["passWifi"] = String(ConfigSettings.password);
        if (ConfigSettings.dhcpWiFi) {
            doc["dchp"] = checked;
        }
        doc["ip"] = ConfigSettings.ipAddressWiFi;
        doc["mask"] = ConfigSettings.ipMaskWiFi;
        doc["gw"] = ConfigSettings.ipGWWiFi;

        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}

void handleSerial() {
        String result;
        DynamicJsonDocument doc(1024);
        // result += F("<html>");
        //  result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     doc["logoutLink"] = LOGOUT_LINK;
        // } else {
        //     doc["logoutLink"] = "";
        // }
        // result += FPSTR(HTTP_SERIAL);
        // result += F("</html>");

        doc["pageName"] = "Config Serial";

        if (ConfigSettings.serialSpeed == 9600) {
            doc["selected9600"] = checked;
        } else if (ConfigSettings.serialSpeed == 19200) {
            doc["selected19200"] = checked;
        } else if (ConfigSettings.serialSpeed == 38400) {
            doc["selected38400"] = checked;
        } else if (ConfigSettings.serialSpeed == 57600) {
            doc["selected57600"] = checked;
        } else if (ConfigSettings.serialSpeed == 115200) {
            doc["selected115200"] = checked;
        } else {
            doc["selected115200"] = checked;
        }
        doc["socketPort"] = String(ConfigSettings.socketPort);

        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}

void handleEther() {
        String result;
        DynamicJsonDocument doc(1024);
        // result += F("<html>");
        //  result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     doc["logoutLink"] = LOGOUT_LINK;
        // } else {
        //     doc["logoutLink"] = "";
        // }
        // result += FPSTR(HTTP_ETHERNET);
        // result += F("</html>");

        doc["pageName"] = "Config Ethernet";

        if (ConfigSettings.dhcp) {
            doc["modeEther"] = checked;
        }
        doc["ipEther"] = ConfigSettings.ipAddress;
        doc["maskEther"] = ConfigSettings.ipMask;
        doc["GWEther"] = ConfigSettings.ipGW;

        if (ConfigSettings.disablePingCtrl) {
            doc["disablePingCtrl"] = checked;
        }

        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}

void handleRoot() {
        String result;
        DynamicJsonDocument doc(1024);
        // result += F("<html>");
        //  result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     // doc["logoutLink}}", LOGOUT_LINK);
        // } else {
        //     // result.replace("{{logoutLink}}", "");
        // }
        // result += FPSTR(HTTP_ROOT);
        // result += F("</html>");

        doc["pageName"] = "Status";
        doc["VERSION"] = VERSION;

        String readableTime;
        getReadableTime(readableTime, ConfigSettings.socketTime);
        if (ConfigSettings.connectedClients > 0) {
            if (ConfigSettings.connectedClients > 1) {
                doc["connectedSocketStatus"] = "Yes, " + String(ConfigSettings.connectedClients) + "connection";
                doc["connectedSocket"] = readableTime;
            } else {
                doc["connectedSocketStatus"], "Yes, " + String(ConfigSettings.connectedClients) + " connections";
                doc["connectedSocket"] = readableTime;
            }
        } else {
            doc["connectedSocketStatus"] = "No";
            doc["connectedSocket"] = "Not connected";
        }

        switch (ConfigSettings.coordinator_mode) {
            case COORDINATOR_MODE_USB:
                doc["operationalMode"] = "Zigbee-to-USB";
                break;
            case COORDINATOR_MODE_WIFI:
                doc["operationalMode"] = "Zigbee-to-WIFI";
                break;
            case COORDINATOR_MODE_LAN:
                doc["operationalMode"] = "Zigbee-to-LAN";
                break;

            default:
                break;
        }

        // ETHERNET TAB
        if (ConfigSettings.dhcp) {
            doc["ethDhcp"] = "On";
            // result.replace("{{ethIp}}", ETH.localIP().toString());
            // result.replace("{{etchMask}}", ETH.subnetMask().toString());
            // result.replace("{{ethGate}}", ETH.gatewayIP().toString());
        } else {
            doc["ethDhcp"] = "Off";
            // result.replace("{{ethIp}}", ConfigSettings.ipAddress);
            // result.replace("{{etchMask}}", ConfigSettings.ipMask);
            // result.replace("{{ethGate}}", ConfigSettings.ipGW);
        }
        if (ConfigSettings.connectedEther) {
            doc["connectedEther"] = "Yes";
            doc["ethConnection"] = "Connected";
            doc["ethMac"] = ETH.macAddress();
            doc["ethSpd"] = String(ETH.linkSpeed()) + String(" Mbps");
            doc["ethIp"] = ETH.localIP().toString();
            doc["etchMask"] = ETH.subnetMask().toString();
            doc["ethGate"] = ETH.gatewayIP().toString();
            // if (ConfigSettings.dhcp) {
            //     result.replace("{{ethDhcp}}", "On");
            //     // result.replace("{{ethIp}}", ETH.localIP().toString());
            //     // result.replace("{{etchMask}}", ETH.subnetMask().toString());
            //     // result.replace("{{ethGate}}", ETH.gatewayIP().toString());
            // } else {
            //     result.replace("{{ethDhcp}}", "Off");
            //     // result.replace("{{ethIp}}", ConfigSettings.ipAddress);
            //     // result.replace("{{etchMask}}", ConfigSettings.ipMask);
            //     // result.replace("{{ethGate}}", ConfigSettings.ipGW);
            // }
        } else {
            doc["connectedEther"] = "No";
            doc["ethConnection"] = "Not connected";
            doc["ethMac"] = "Not connected";
            doc["ethSpd"] = "Not connected";
            doc["ethIp"] = "Not connected";
            doc["etchMask"] = "Not connected";
            doc["ethGate"] = "Not connected";
            // if (ConfigSettings.dhcp) {
            //     result.replace("{{ethDhcp}}", "On");
            //     result.replace("{{ethIp}}", "Not connected");
            //     result.replace("{{etchMask}}", "Not connected");
            //     result.replace("{{ethGate}}", "Not connected");
            // } else {
            //     result.replace("{{ethDhcp}}", "Off");
            //     result.replace("{{ethIp}}", ConfigSettings.ipAddress);
            //     result.replace("{{etchMask}}", ConfigSettings.ipMask);
            //     result.replace("{{ethGate}}", ConfigSettings.ipGW);
            // }
        }

        getReadableTime(readableTime, 0);
        doc["uptime"] = readableTime;

        float CPUtemp = getCPUtemp();
        doc["deviceTemp"] = String(CPUtemp);
        doc["hwRev"] = BOARD_NAME;
        doc["espModel"] = String(ESP.getChipModel());
        doc["espCores"] = String(ESP.getChipCores());
        doc["espFreq"] = String(ESP.getCpuFreqMHz());
        doc["espHeapFree"] = String(ESP.getFreeHeap() / 1024);
        doc["espHeapSize"] = String(ESP.getHeapSize() / 1024);

        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        if (chip_info.features & CHIP_FEATURE_EMB_FLASH) {
            doc["espFlashType"] = "embedded";
        } else {
            doc["espFlashType"] = "external";
        }

        doc["espFlashSize"] = String(ESP.getFlashChipSize() / (1024 * 1024));

        if (ConfigSettings.coordinator_mode == COORDINATOR_MODE_WIFI) {
            doc["wifiEnabled"] = "Yes";

            if (WiFi.status() == WL_CONNECTED) {  // STA connected
                String rssiWifi = String(WiFi.RSSI()) + String(" dBm");
                doc["wifiSsid"] = WiFi.SSID();
                doc["wifiMac"] = WiFi.macAddress().c_str();
                doc["wifiRssi"] = rssiWifi;
                doc["wifiConnected"] = "Connected to " + WiFi.SSID();
                doc["wifiIp"] = WiFi.localIP().toString();
                doc["wifiSubnet"] = WiFi.subnetMask().toString();
                doc["wifiGate"] = WiFi.gatewayIP().toString();
            } else {
                const char *connecting = "Connecting...";
                doc["wifiSsid"] = ConfigSettings.ssid;
                doc["wifiMac"] = WiFi.macAddress().c_str();
                doc["wifiRssi"] = connecting;
                doc["wifiConnected"] = connecting;
                doc["wifiIp"] = ConfigSettings.dhcpWiFi ? WiFi.localIP().toString() : connecting;
                doc["wifiSubnet"] = ConfigSettings.dhcpWiFi ? WiFi.subnetMask().toString() : connecting;
                doc["wifiGate"] = ConfigSettings.dhcpWiFi ? WiFi.gatewayIP().toString() : connecting;
            }

            if (ConfigSettings.dhcpWiFi) {
                doc["wifiDhcp"] = "On";
            } else {
                doc["wifiDhcp"] = "Off";
            }
        } else {
            doc["wifiEnabled"] = "No";
            doc["wifiConnected"] = "Not connected";
            doc["wifiMode"] = "Off";
            doc["wifiSsid"] = "Off";
            doc["wifiMac"] = "Off";
            doc["wifiIp"] = "Off";
            doc["wifiSubnet"] = "Off";
            doc["wifiGate"] = "Off";
            doc["wifiRssi"] = "Off";
            doc["wifiDhcp"] = "Off";
        }
        if (WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {  // AP active
            doc["wifiIp"] = WiFi.localIP().toString();
            String AP_NameString;
            getDeviceID(AP_NameString);
            doc["wifiSsid"] = AP_NameString + "] = no password";
            doc["wifiIp"] = "192.168.1.1 (SLZB-06 web interface)";
            doc["wifiSubnet"] = "255.255.255.0 (Access point)";
            doc["wifiGate"] = "192.168.1.1 (this device)";
            doc["wifiDhcp"] = "On (Access point)";
            doc["wifiModeAPStatus"] = "AP started";
            doc["wifiMode"] = "AP";
            doc["wifiModeAP"] = "Yes";
        } else {
            doc["wifiModeAP"] = "No";
            doc["wifiModeAPStatus"] = "Not started";
            doc["wifiMode"] = "Client";
        }
        // if (ConfigSettings.disableEmerg) {
        //     result.replace("{{wifiModeAP}}", "No");
        //     result.replace("{{wifiModeAPStatus}}", "Disabled");
        // } else {
        //     result.replace("{{wifiModeAP}}", "Yes");
        //     result.replace("{{wifiModeAPStatus}}", "Not started");
        // }
        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}

// void handleSaveGeneral() {
//     if (checkAuth()) {
//         String StringConfig;
//         String refreshLogs;
//         String hostname;
//         String usbMode;
//         String disableLedYellow;
//         String disableLedBlue;

//         if (serverWeb.arg("refreshLogs").toDouble() < 1000) {
//             refreshLogs = "1000";
//         } else {
//             refreshLogs = serverWeb.arg("refreshLogs");
//         }

//         if (serverWeb.arg("hostname") != "") {
//             hostname = serverWeb.arg("hostname");
//         } else {
//             hostname = "SLZB-06";
//         }
//         // DEBUG_PRINTLN(hostname);

//         if (serverWeb.hasArg(coordMode)) {
//             printLogMsg(F("[SaveGeneral] 'coordMode' arg is:"));
//             printLogMsg(serverWeb.arg(coordMode));
//             ConfigSettings.coordinator_mode = static_cast<COORDINATOR_MODE_t>(serverWeb.arg(coordMode).toInt());
//             printLogMsg(F("[SaveGeneral] 'static_cast' res is:"));
//             printLogMsg(String(ConfigSettings.coordinator_mode));
//         } else {
//             printLogMsg(F("[SaveGeneral] no 'coordMode' arg"));
//         }

//         if (serverWeb.arg("disableLedYellow") == "on") {
//             disableLedYellow = "1";
//         } else {
//             disableLedYellow = "0";
//             ConfigSettings.disableLeds = false;
//         }
//         if (serverWeb.arg("disableLedBlue") == "on") {
//             disableLedBlue = "1";
//         } else {
//             disableLedBlue = "0";
//             ConfigSettings.disableLeds = false;
//         }
//         const char *path = "/config/configGeneral.json";

//         StringConfig = "{\"refreshLogs\":" + refreshLogs + ",\"hostname\":\"" + hostname + "\",\"disableLeds\": " + String(ConfigSettings.disableLeds) + ",\"usbMode\":\"" + usbMode + "\",\"disableLedYellow\":\"" + disableLedYellow + "\",\"disableLedBlue\":" + disableLedBlue + ",\"" + coordMode + "\":" + String(ConfigSettings.coordinator_mode) + ",\"" + prevCoordMode + "\":" + String(ConfigSettings.prevCoordinator_mode) + "}";
//         DEBUG_PRINTLN(StringConfig);
//         DynamicJsonDocument doc(1024);
//         deserializeJson(doc, StringConfig);

//         File configFile = LittleFS.open(path, FILE_WRITE);
//         if (!configFile) {
//             DEBUG_PRINTLN(F("failed open"));
//         } else {
//             serializeJson(doc, configFile);
//         }
//         handleSaveSucces("config");
//     }
// }

// void handleSaveSecurity() {
//     if (checkAuth()) {
//         String StringConfig;
//         String disableWeb;
//         String webAuth;
//         String webUser;
//         String webPass;

//         if (serverWeb.arg("disableWeb") == "on") {
//             disableWeb = "1";
//         } else {
//             disableWeb = "0";
//         }

//         if (serverWeb.arg("webAuth") == "on") {
//             webAuth = "1";
//         } else {
//             webAuth = "0";
//         }

//         if (serverWeb.arg("webUser") != "") {
//             webUser = serverWeb.arg("webUser");
//         } else {
//             webUser = "admin";
//         }

//         if (serverWeb.arg("webPass") != "") {
//             webPass = serverWeb.arg("webPass");
//         } else {
//             webPass = "admin";
//         }

//         // DEBUG_PRINTLN(hostname);
//         const char *path = "/config/configSecurity.json";

//         StringConfig = "{\"disableWeb\":" + disableWeb + ",\"webAuth\":" + webAuth + ",\"webUser\":\"" + webUser + "\",\"webPass\":\"" + webPass + "\"}";
//         DEBUG_PRINTLN(StringConfig);
//         DynamicJsonDocument doc(1024);
//         deserializeJson(doc, StringConfig);

//         File configFile = LittleFS.open(path, FILE_WRITE);
//         if (!configFile) {
//             DEBUG_PRINTLN(F("failed open"));
//         } else {
//             serializeJson(doc, configFile);
//         }
//         handleSaveSucces("config");
//     }
// }

// void handleSaveWifi() {
//     if (checkAuth()) {
//         if (!serverWeb.hasArg("WIFISSID")) {
//             serverWeb.send(500, contTypeText, "BAD ARGS");
//             return;
//         }

//         String StringConfig;
//         String enableWiFi;
//         if (serverWeb.arg("wifiEnable") == "on") {
//             enableWiFi = "1";
//         } else {
//             enableWiFi = "0";
//         }
//         String ssid = serverWeb.arg("WIFISSID");
//         String pass = serverWeb.arg("WIFIpassword");

//         String dhcpWiFi;
//         if (serverWeb.arg("dhcpWiFi") == "on") {
//             dhcpWiFi = "1";
//         } else {
//             dhcpWiFi = "0";
//         }

//         String ipAddress = serverWeb.arg("ipAddress");
//         String ipMask = serverWeb.arg("ipMask");
//         String ipGW = serverWeb.arg("ipGW");
//         // String disableEmerg;
//         // if (serverWeb.arg("disableEmerg") == "on") {
//         //     disableEmerg = "1";
//         //     saveEmergencyWifi(0);
//         // } else {
//         //     disableEmerg = "0";
//         // }
//         const char *path = "/config/configWifi.json";

//         StringConfig = "{\"enableWiFi\":" + enableWiFi + ",\"ssid\":\"" + ssid + "\",\"pass\":\"" + pass + "\",\"dhcpWiFi\":" + dhcpWiFi + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\"}";
//         DEBUG_PRINTLN(StringConfig);
//         DynamicJsonDocument doc(1024);
//         deserializeJson(doc, StringConfig);

//         File configFile = LittleFS.open(path, FILE_WRITE);
//         if (!configFile) {
//             DEBUG_PRINTLN(F("failed open"));
//         } else {
//             serializeJson(doc, configFile);
//         }
//         handleSaveSucces("config");
//     }
// }

// void handleSaveSerial() {
//     if (checkAuth()) {
//         String StringConfig;
//         String serialSpeed = serverWeb.arg("baud");
//         String socketPort = serverWeb.arg("port");
//         const char *path = "/config/configSerial.json";

//         StringConfig = "{\"baud\":" + serialSpeed + ", \"port\":" + socketPort + "}";
//         DEBUG_PRINTLN(StringConfig);
//         DynamicJsonDocument doc(1024);
//         deserializeJson(doc, StringConfig);

//         File configFile = LittleFS.open(path, FILE_WRITE);
//         if (!configFile) {
//             DEBUG_PRINTLN(F("failed open"));
//         } else {
//             serializeJson(doc, configFile);
//         }
//         handleSaveSucces("config");
//     }
// }

// void handleSaveEther() {
//     if (checkAuth()) {
//         if (!serverWeb.hasArg("ipAddress")) {
//             serverWeb.send(500, contTypeText, "BAD ARGS");
//             return;
//         }

//         String StringConfig;
//         String dhcp;
//         if (serverWeb.arg("dhcp") == "on") {
//             dhcp = "1";
//         } else {
//             dhcp = "0";
//         }
//         String ipAddress = serverWeb.arg("ipAddress");
//         String ipMask = serverWeb.arg("ipMask");
//         String ipGW = serverWeb.arg("ipGW");

//         String disablePingCtrl;
//         if (serverWeb.arg("disablePingCtrl") == "on") {
//             disablePingCtrl = "1";
//         } else {
//             disablePingCtrl = "0";
//         }

//         const char *path = "/config/configEther.json";

//         StringConfig = "{\"dhcp\":" + dhcp + ",\"ip\":\"" + ipAddress + "\",\"mask\":\"" + ipMask + "\",\"gw\":\"" + ipGW + "\",\"disablePingCtrl\":\"" + disablePingCtrl + "\"}";
//         DEBUG_PRINTLN(StringConfig);
//         DynamicJsonDocument doc(1024);
//         deserializeJson(doc, StringConfig);

//         File configFile = LittleFS.open(path, FILE_WRITE);
//         if (!configFile) {
//             DEBUG_PRINTLN(F("failed open"));
//         } else {
//             serializeJson(doc, configFile);
//         }
//         handleSaveSucces("config");
//     }
// }

void handleReboot() {//todo remove
    if (checkAuth()) {
        String result;

        result += F("<html>");
        result += F("<meta http-equiv='refresh' content='1; URL=/'>");
        // result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     result.replace("{{logoutLink}}", LOGOUT_LINK);
        // } else {
        //     result.replace("{{logoutLink}}", "");
        // }
        result += F("<div class='col py-3'>");
        result += F("<h2>{{pageName}}</h2>");
        result += F("</div></div></div>");
        result = result + F("</body></html>");
        result.replace("{{pageName}}", "Rebooted");

        serverWeb.send(200, F("text/html"), result);

        ESP.restart();
    }
}

// void handleUpdate() {
//     if (checkAuth()) {
//         String result;
//         result += F("<html>");
//         // result += FPSTR(HTTP_HEADER);
//         if (ConfigSettings.webAuth) {
//             result.replace("{{logoutLink}}", LOGOUT_LINK);
//         } else {
//             result.replace("{{logoutLink}}", "");
//         }
//         result += F("<h2>{{pageName}}</h2>");
//         result += F("<div class='btn-group-vertical'>");
//         result += F("<a href='/setchipid' class='btn btn-primary mb-2'>setChipId</button>");
//         result += F("<a href='/setmodeprod' class='btn btn-primary mb-2'>setModeProd</button>");
//         result += F("</div>");

//         result = result + F("</body></html>");
//         result.replace("{{pageName}}", "Update Zigbee");

//         serverWeb.send(200, F("text/html"), result);
//     }
// }

void handleSysTools() {
        String result;
        DynamicJsonDocument doc(512);
        // result += F("<html>");
        //  result += FPSTR(HTTP_HEADER);
        // if (ConfigSettings.webAuth) {
        //     doc["logoutLink"] = LOGOUT_LINK;
        // } else {
        //     doc["logoutLink"] = "";
        // }
        // result += F("<div class='col py-3'>");
        // result += F("<h2>{{pageName}}</h2>");
        // result += FPSTR(HTTP_SYSTOOLS);
        // result += F("</div></div>");
        // result = result + F("</body></html>");
        doc["pageName"] = "System and Tools";
        doc["hostname"] = ConfigSettings.hostname;
        doc["refreshLogs"] = ConfigSettings.refreshLogs;
        serializeJson(doc, result);
        serverWeb.sendHeader(respHeaderName, result);
}

// void handleReadfile() {
//     if (checkAuth()) {
//         String result;
//         String filename = "/config/" + serverWeb.arg(0);
//         File file = LittleFS.open(filename, "r");

//         if (!file) {
//             return;
//         }

//         while (file.available()) {
//             result += (char)file.read();
//         }
//         file.close();

//         serverWeb.send(200, F("text/html"), result);
//     }
// }

void handleSavefile() {
    if (checkAuth()) {
        if (serverWeb.method() != HTTP_POST) {
            serverWeb.send(405, contTypeText, F("Method Not Allowed"));
        } else {
            String filename = "/config/" + serverWeb.arg(0);
            String content = serverWeb.arg(1);
            File file = LittleFS.open(filename, "w");
            if (!file) {
                DEBUG_PRINT(F("Failed to open file for reading\r\n"));
                return;
            }

            int bytesWritten = file.print(content);

            if (bytesWritten > 0) {
                DEBUG_PRINTLN(F("File was written"));
                DEBUG_PRINTLN(bytesWritten);
            } else {
                DEBUG_PRINTLN(F("File write failed"));
            }

            file.close();
            serverWeb.sendHeader(F("Location"), F("/sys-tools"));
            serverWeb.send(303);
        }
    }
}

void handleLogBuffer() {
    if (checkAuth()) {
        String result;
        result = logPrint();

        serverWeb.send(HTTP_CODE_OK, contTypeText, result);
    }
}

// void handleScanNetworkDone() {
//     static uint8_t timeout = 0;
//     DynamicJsonDocument doc(1024);
//     String result = "";
//     int16_t scanRes = WiFi.scanComplete();
//     doc["scanDone"] = false;
//     if (scanRes == -2) {
//         WiFi.scanNetworks(true);
//     } else if (scanRes > 0) {
//         doc["scanDone"] = true;
//         JsonArray wifi = doc.createNestedArray("wifi");
//         for (int i = 0; i < scanRes; ++i) {
//             JsonObject wifi_0 = wifi.createNestedObject();
//             wifi_0["ssid"] = WiFi.SSID(i);
//             wifi_0["rssi"] = WiFi.RSSI(i);
//             wifi_0["channel"] = WiFi.channel(i);
//             wifi_0["secure"] = WiFi.encryptionType(i);
//         }
//         WiFi.scanDelete();
//         if (ConfigSettings.coordinator_mode == COORDINATOR_MODE_LAN) WiFi.mode(WIFI_OFF);
//     }
//     if (timeout < 5) {
//         timeout++;
//     } else {
//         doc["scanDone"] = true;
//         WiFi.scanDelete();
//         timeout = 0;
//         if (ConfigSettings.coordinator_mode == COORDINATOR_MODE_LAN) WiFi.mode(WIFI_OFF);
//     }

//     serializeJson(doc, result);
//     serverWeb.send(200, F("application/json"), result);
// }

// void handleScanNetwork() {
//     if (checkAuth()) {
//         if (WiFi.getMode() == WIFI_OFF) {  // enable wifi for scan
//             WiFi.mode(WIFI_STA);
//             WiFi.scanNetworks(true);
//         } else if (WiFi.getMode() == WIFI_AP) {  // enable sta for scan
//             WiFi.mode(WIFI_AP_STA);
//         }
//         serverWeb.send(200, F("text/html"), "ok");
//     }
// }

void handleClearConsole() {//todo move to api
    if (checkAuth()) {
        logClear();

        serverWeb.send(HTTP_CODE_OK, contTypeText, "ok");
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

void handleZigbeeRestart() {
    if (checkAuth()) {
        serverWeb.send(HTTP_CODE_OK, contTypeText, "");

        zigbeeRestart();
    }
}

void handleZigbeeBSL() {
    if (checkAuth()) {
        serverWeb.send(HTTP_CODE_OK, contTypeText, "");

        zigbeeEnableBSL();
    }
}

void handleAdapterModeUSB() {
    if (checkAuth()) {
        serverWeb.send(HTTP_CODE_OK, contTypeText, "");

        adapterModeUSB();
    }
}

void handleAdapterModeLAN() {
    if (checkAuth()) {
        serverWeb.send(HTTP_CODE_OK, contTypeText, "");

        adapterModeLAN();
    }
}

void handleLedYellowToggle() {
    if (checkAuth()) {
        serverWeb.send(HTTP_CODE_OK, contTypeText, "");

        ledYellowToggle();
    }
}

void handleLedBlueToggle() {
    if (checkAuth()) {
        serverWeb.send(HTTP_CODE_OK, contTypeText, "");

        ledBlueToggle();
    }
}

void printLogTime() {
    String tmpTime;
    unsigned long timeLog = millis();
    tmpTime = String(timeLog, DEC);
    logPush('[');
    for (int j = 0; j < tmpTime.length(); j++) {
        logPush(tmpTime[j]);
    }
    logPush(']');
}

void printLogMsg(String msg) {
    printLogTime();
    logPush(' ');
    logPush('|');
    logPush(' ');
    for (int j = 0; j < msg.length(); j++) {
        logPush(msg[j]);
    }
    logPush('\n');
}

// void handleWEBUpdate() {
//     if (checkAuth()) {
//         String result;
//         result += F("<html>");
//         // result += FPSTR(HTTP_HEADER);
//         if (ConfigSettings.webAuth) {
//             result.replace("{{logoutLink}}", LOGOUT_LINK);
//         } else {
//             result.replace("{{logoutLink}}", "");
//         }

//         result += F("<h2>{{pageName}}</h2>");
//         // result += FPSTR(HTTP_SYSTOOLS);
//         result = result + F("</body></html>");
//         result.replace("{{pageName}}", "Update ESP32");
//         serverWeb.sendHeader("Connection", "close");

//         serverWeb.send(200, "text/html", result);
//         checkUpdateFirmware();
//     }
// }

int totalLength;        // total size of firmware
int currentLength = 0;  // current size of written firmware

void progressFunc(unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u of %u\r", progress, total);
};

void checkUpdateFirmware() {
    clientWeb.begin(UPD_FILE);
    clientWeb.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    // Get file, just to check if each reachable
    int resp = clientWeb.GET();
    Serial.print("Response: ");
    Serial.println(resp);
    // If file is reachable, start downloading
    if (resp == HTTP_CODE_OK) {
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
        uint8_t buff[128] = {0};
        // get tcp stream
        WiFiClient *stream = clientWeb.getStreamPtr();
        // read all data from server
        DEBUG_PRINTLN("Updating firmware...");
        while (clientWeb.connected() && (len > 0 || len == -1)) {
            // get available data size
            size_t size = stream->available();
            if (size) {
                // read up to 128 byte
                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                // pass to function
                runUpdateFirmware(buff, c);
                if (len > 0) {
                    len -= c;
                }
            }
            // DEBUG_PRINT("Bytes left to flash ");
            // DEBUG_PRINTLN(len);
            // delay(1);
        }
    } else {
        Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in UPD_FILE.");
    }
    clientWeb.end();
}

void runUpdateFirmware(uint8_t *data, size_t len) {
    Update.write(data, len);
    currentLength += len;
    // Print dots while waiting for update to finish
    Serial.print('.');
    // if current length of written firmware is not equal to total firmware size, repeat
    if (currentLength != totalLength) return;
    Update.end(true);
    Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
    // Restart ESP32 to see changes
    ESP.restart();
}