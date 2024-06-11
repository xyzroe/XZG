#include <Arduino.h>
#include <ArduinoJson.h>
#include <ETH.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <Update.h>
#include <WebServer.h>
#include <FS.h>
#include <WiFi.h>
#include <Ticker.h>
#include <CCTools.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "zb.h"
#include "const/zones.h"
#include "const/keys.h"
// #include "const/hw.h"

#include "webh/html/PAGE_VPN.html.gz.h"
#include "webh/html/PAGE_MQTT.html.gz.h"
#include "webh/html/PAGE_ABOUT.html.gz.h"
#include "webh/html/PAGE_GENERAL.html.gz.h"
#include "webh/html/PAGE_LOADER.html.gz.h"
#include "webh/html/PAGE_ROOT.html.gz.h"
#include "webh/html/PAGE_SECURITY.html.gz.h"
#include "webh/html/PAGE_ZIGBEE.html.gz.h"
#include "webh/html/PAGE_TOOLS.html.gz.h"
#include "webh/html/PAGE_NETWORK.html.gz.h"
#include "webh/html/PAGE_LOGIN.html.gz.h"

#include "webh/js/i18next.min.js.gz.h"
#include "webh/js/i18nextHttpBackend.min.js.gz.h"
#include "webh/js/functions.js.gz.h"
#include "webh/js/bootstrap.bundle.min.js.gz.h"
#include "webh/js/jquery-min.js.gz.h"
#include "webh/js/masonry.js.gz.h"

#include "webh/css/style.css.gz.h"
#include "webh/img/icons.svg.gz.h"
#include "webh/img/logo.svg.gz.h"

#include "webh/json/en.json.gz.h"
#include "webh/json/uk.json.gz.h"
#include "webh/json/zh.json.gz.h"
#include "webh/json/es.json.gz.h"
#include "webh/json/pt.json.gz.h"
#include "webh/json/ru.json.gz.h"
#include "webh/json/fr.json.gz.h"
#include "webh/json/de.json.gz.h"
#include "webh/json/ja.json.gz.h"
#include "webh/json/tr.json.gz.h"
#include "webh/json/it.json.gz.h"
#include "webh/json/pl.json.gz.h"
#include "webh/json/cz.json.gz.h"

// #define HTTP_DOWNLOAD_UNIT_SIZE 3000

// #define HTTP_UPLOAD_BUFLEN 3000

// #define HTTP_MAX_DATA_WAIT 10000 // ms to wait for the client to send the request
// #define HTTP_MAX_POST_WAIT 10000 // ms to wait for POST data to arrive
// #define HTTP_MAX_SEND_WAIT 10000 // ms to wait for data chunk to be ACKed
// #define HTTP_MAX_CLOSE_WAIT 4000 // ms to wait for the client to close the connection

extern CCTools CCTool;

extern struct SysVarsStruct vars;
extern struct ThisConfigStruct hwConfig;
extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct VpnConfigStruct vpnCfg;
extern struct MqttConfigStruct mqttCfg;

extern LEDControl ledControl;

extern IPAddress apIP;

bool wifiWebSetupInProgress = false;

bool eventOK = false;
// extern const char *coordMode;

// extern const char *deviceModel;

const char *contTypeTextHtml = "text/html";
const char *contTypeTextJs = "text/javascript";
const char *contTypeTextCss = "text/css";
const char *contTypeTextSvg = "image/svg+xml";
const char *checked = "true";
const char *respHeaderName = "respValuesArr";
const char *respTimeZonesName = "respTimeZones";
const char *contTypeJson = "application/json";
const char *contTypeText = "text/plain";

const char *tempFile = "/fw.hex";

bool opened = false;
File fwFile;

extern bool loadFileConfigMqtt();
extern bool loadFileConfigWg();

WebServer serverWeb(80);

WiFiClient eventsClient;

void webServerHandleClient()
{
    serverWeb.handleClient();
}

void redirectLogin(int msg = 0)
{
    String path = "/login";
    if (msg != 0)
    {
        path = path + "?msg=" + msg;
    }

    serverWeb.sendHeader("Location", path);
    serverWeb.sendHeader("Cache-Control", "no-cache");
    serverWeb.send(301);
}

void handleLoader()
{
    if (captivePortal())
    {
        return;
    }
    if (!is_authenticated())
    {
        redirectLogin();
        return;
    }
    sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len);
}

void initWebServer()
{
    /* ----- LANG FILES | START -----*/
    serverWeb.on("/lg/en.json", []()
                 { sendGzip(contTypeJson, en_json_gz, en_json_gz_len); });
    serverWeb.on("/lg/uk.json", []()
                 { sendGzip(contTypeJson, uk_json_gz, uk_json_gz_len); });
    serverWeb.on("/lg/zh.json", []()
                 { sendGzip(contTypeJson, zh_json_gz, zh_json_gz_len); });
    serverWeb.on("/lg/es.json", []()
                 { sendGzip(contTypeJson, es_json_gz, es_json_gz_len); });
    serverWeb.on("/lg/pt.json", []()
                 { sendGzip(contTypeJson, pt_json_gz, pt_json_gz_len); });
    serverWeb.on("/lg/ru.json", []()
                 { sendGzip(contTypeJson, ru_json_gz, ru_json_gz_len); });
    serverWeb.on("/lg/fr.json", []()
                 { sendGzip(contTypeJson, fr_json_gz, fr_json_gz_len); });
    serverWeb.on("/lg/de.json", []()
                 { sendGzip(contTypeJson, de_json_gz, de_json_gz_len); });
    serverWeb.on("/lg/ja.json", []()
                 { sendGzip(contTypeJson, ja_json_gz, ja_json_gz_len); });
    serverWeb.on("/lg/tr.json", []()
                 { sendGzip(contTypeJson, tr_json_gz, tr_json_gz_len); });
    serverWeb.on("/lg/it.json", []()
                 { sendGzip(contTypeJson, it_json_gz, it_json_gz_len); });
    serverWeb.on("/lg/pl.json", []()
                 { sendGzip(contTypeJson, pl_json_gz, pl_json_gz_len); });
    serverWeb.on("/lg/cz.json", []()
                 { sendGzip(contTypeJson, cz_json_gz, cz_json_gz_len); });
    /* ----- LANG FILES | END -----*/
    /* ----- JS and CSS FILES | START -----*/
    serverWeb.on("/js/i18next.min.js", []()
                 { sendGzip(contTypeTextJs, i18next_min_js_gz, i18next_min_js_gz_len); });
    serverWeb.on("/js/i18nextHttpBackend.min.js", []()
                 { sendGzip(contTypeTextJs, i18nextHttpBackend_min_js_gz, i18nextHttpBackend_min_js_gz_len); });
    serverWeb.on("/js/bootstrap.bundle.min.js", []()
                 { sendGzip(contTypeTextJs, bootstrap_bundle_min_js_gz, bootstrap_bundle_min_js_gz_len); });
    serverWeb.on("/js/masonry.js", []()
                 { sendGzip(contTypeTextJs, masonry_js_gz, masonry_js_gz_len); });
    serverWeb.on("/js/functions.js", []()
                 { sendGzip(contTypeTextJs, functions_js_gz, functions_js_gz_len); });
    serverWeb.on("/js/jquery-min.js", []()
                 { sendGzip(contTypeTextJs, jquery_min_js_gz, jquery_min_js_gz_len); });
    serverWeb.on("/css/style.css", []()
                 { sendGzip(contTypeTextCss, required_css_gz, required_css_gz_len); });
    /* ----- JS and CSS FILES  | END -----*/
    /* ----- SVG FILES | START -----*/
    serverWeb.on("/logo.svg", []()
                 { sendGzip(contTypeTextSvg, logo_svg_gz, logo_svg_gz_len); });
    serverWeb.on("/icons.svg", []()
                 { sendGzip(contTypeTextSvg, icons_svg_gz, icons_svg_gz_len); });

    serverWeb.onNotFound(handleNotFound);

    /* ----- SVG FILES | END -----*/
    /* ----- PAGES | START -----*/
    serverWeb.on("/", handleLoader);
    serverWeb.on("/generate_204", handleLoader);
    serverWeb.on("/general", handleLoader);
    serverWeb.on("/security", handleLoader);
    serverWeb.on("/network", handleLoader);
    serverWeb.on("/ethernet", handleLoader);
    serverWeb.on("/zigbee", handleLoader);
    serverWeb.on("/about", handleLoader);
    serverWeb.on("/tools", handleLoader);
    serverWeb.on("/mqtt", handleLoader);
    serverWeb.on("/vpn", handleLoader);
    serverWeb.on("/login", []()
                 { if (serverWeb.method() == HTTP_GET) {
                        handleLoginGet();
                    } else if (serverWeb.method() == HTTP_POST) {
                        handleLoginPost();
                    } });
    serverWeb.on("/logout", HTTP_GET, handleLogout);
    /* ----- PAGES | END -----*/
    /* ----- MIST CMDs | END -----*/
    serverWeb.on("/saveParams", HTTP_POST, handleSaveParams);
    serverWeb.on("/cmdZigRST", handleZigbeeRestart);
    serverWeb.on("/cmdZigBSL", handleZigbeeBSL);
    serverWeb.on("/saveFile", handleSavefile);

    serverWeb.on("/api", handleApi);

    serverWeb.on("/events", handleEvents);
    /* ----- MIST CMDs | END -----*/

    /* ----- OTA | START -----*/

    serverWeb.on("/update", HTTP_POST, handleUpdateRequest, handleEspUpdateUpload);
    // serverWeb.on("/updateZB", HTTP_POST, handleUpdateRequest, handleZbUpdateUpload);

    /* ----- OTA | END -----*/

    const char *headerkeys[] = {"Content-Length", "Cookie"};
    size_t headerkeyssize = sizeof(headerkeys) / sizeof(char *);
    serverWeb.collectHeaders(headerkeys, headerkeyssize);
    serverWeb.begin();
    LOGD("done");
}

// IPAddress apIP2(192, 168, 1, 1);

bool captivePortal()
{
    if (vars.apStarted)
    {
        IPAddress ip;
        if (!ip.fromString(serverWeb.hostHeader()))
        {
            LOGD("Request redirected to captive portal");
            serverWeb.sendHeader("Location", String("http://") + apIP.toString(), true);
            serverWeb.send(302, "text/plain", "");
            serverWeb.client().stop();
            return true;
        }
    }
    return false;
}

/*
void handleNotFound()
{
    if (vars.apStarted)
    {
        sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len);
    }
    else
    {
        serverWeb.send(HTTP_CODE_OK, contTypeText, F("URL NOT FOUND"));
    }
}
*/

void handleNotFound()
{
    if (captivePortal())
    {
        return;
    }
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
        message += String(F(" ")) + serverWeb.argName(i) + F(": ") + serverWeb.arg(i) + F("\n");
    }
    serverWeb.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    serverWeb.sendHeader("Pragma", "no-cache");
    serverWeb.sendHeader("Expires", "-1");
    serverWeb.send(404, "text/plain", message);
}

void handleUpdateRequest()
{
    serverWeb.sendHeader("Connection", "close");
    serverWeb.send(HTTP_CODE_OK, contTypeText, "Upload OK. Try to flash...");
}

void handleEspUpdateUpload()
{
    if (!is_authenticated())
    {
        return;
    }

    HTTPUpload &upload = serverWeb.upload();
    static long contentLength = 0;
    if (upload.status == UPLOAD_FILE_START)
    {
        LOGD("hostHeader: %s", serverWeb.hostHeader());
        contentLength = serverWeb.header("Content-Length").toInt();
        LOGD("contentLength: %s", String(contentLength));
        LOGD("Update ESP from file %s size: %s", String(upload.filename.c_str()), String(upload.totalSize));
        LOGD("upload.currentSize %s", String(upload.currentSize));
        if (!Update.begin(contentLength))
        {
            Update.printError(Serial);
        }
        Update.onProgress(progressFunc);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Update.printError(Serial);
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {
            LOGD("Update success. Rebooting...");
            ESP.restart();
        }
        else
        {
            LOGD("Update error: ");
            Update.printError(Serial);
        }
    }
}

void handleEvents()
{
    if (is_authenticated())
    {
        if (eventsClient)
        {
            eventsClient.stop();
        }
        eventsClient = serverWeb.client();
        if (eventsClient)
        {
            eventsClient.println("HTTP/1.1 200 OK");
            eventsClient.println("Content-Type: text/event-stream;");
            eventsClient.println("Connection: close");
            eventsClient.println("Access-Control-Allow-Origin: *");
            eventsClient.println("Cache-Control: no-cache");
            eventsClient.println();
            eventsClient.flush();
        }
    }
}

void sendEvent(const char *event, const uint8_t evsz, const String data)
{
    if (eventsClient)
    {
        char evnmArr[10 + evsz];
        snprintf(evnmArr, sizeof(evnmArr), "event: %s\n", event);
        eventsClient.print(evnmArr);
        eventsClient.print(String("data: ") + data + "\n\n");
        eventsClient.flush();
    }
}

void sendGzip(const char *contentType, const uint8_t content[], uint16_t contentLen)
{
    serverWeb.sendHeader(F("Content-Encoding"), F("gzip"));
    serverWeb.send_P(HTTP_CODE_OK, contentType, (const char *)content, contentLen);
}

void hex2bin(uint8_t *out, const char *in)
{
    // uint8_t sz = 0;
    while (*in)
    {
        while (*in == ' ')
            in++; // skip spaces
        if (!*in)
            break;
        uint8_t c = *in >= 'a' ? *in - 'a' + 10 : *in >= 'A' ? *in - 'A' + 10
                                                             : *in - '0';
        in++;
        c <<= 4;
        if (!*in)
            break;
        c |= *in >= 'a' ? *in - 'a' + 10 : *in >= 'A' ? *in - 'A' + 10
                                                      : *in - '0';
        in++;
        *out++ = c;
        // sz++;
    }
}

void handleApi()
{ // http://xzg.local/api?action=0&page=0
    enum API_ACTION_t : uint8_t
    {
        API_GET_PAGE,
        API_GET_PARAM,
        API_STARTWIFISCAN,
        API_WIFISCANSTATUS,
        API_GET_FILELIST,
        API_GET_FILE,
        API_SEND_HEX,
        API_WIFICONNECTSTAT,
        API_CMD,
        API_GET_LOG,
        API_DEL_FILE //,
        // API_FLASH_ZB
    };
    const char *action = "action";
    const char *page = "page";
    // const char *Authentication = "Authentication";
    const char *param = "param";
    const char *wrongArgs = "wrong args";
    const char *ok = "ok";
    const char *argFilename = "filename";

    if (!is_authenticated())
    {
        redirectLogin(1);
        return;
    }

    if (serverWeb.argName(0) != action)
    {
        serverWeb.send(500, contTypeText, wrongArgs);
    }
    else
    {
        const uint8_t action = serverWeb.arg(action).toInt();
        switch (action)
        {
        case API_GET_LOG:
        {
            String result;
            result = logPrint();
            serverWeb.send(HTTP_CODE_OK, contTypeText, result);
        }
        break;
        case API_CMD:
        {
            enum CMD_t : uint8_t
            { // cmd list for buttons starts from 0
                CMD_ZB_ROUTER_RECON,
                CMD_ZB_RST,
                CMD_ZB_BSL,
                CMD_ESP_RES,
                CMD_ADAP_LAN,
                CMD_ADAP_USB,
                CMD_LED_ACT,
                CMD_ZB_FLASH,
                CMD_CLEAR_LOG,
                CMD_ESP_UPD_URL,
                CMD_ZB_CHK_FW,
                CMD_ZB_CHK_HW,
                CMD_ZB_LED_TOG,
                CMD_ESP_FAC_RES,
                CMD_ZB_ERASE_NVRAM,
                CMD_DNS_CHECK,
                CMD_BRD_NAME
            };
            String result = wrongArgs;
            const char *argCmd = "cmd";
            const char *argUrl = "url";
            const char *argConf = "conf";
            const char *argLed = "led";
            const char *argAct = "act";
            const char *errLink = "Error getting link";

            if (serverWeb.hasArg(argCmd))
            {
                result = "ok";
                switch (serverWeb.arg(argCmd).toInt())
                {
                case CMD_CLEAR_LOG:
                    logClear();
                    break;
                case CMD_ZB_ROUTER_RECON:
                    zigbeeRouterRejoin();
                    break;
                case CMD_ZB_RST:
                    zigbeeRestart();
                    break;
                case CMD_ZB_BSL:
                    zigbeeEnableBSL();
                    break;
                case CMD_DNS_CHECK:
                    checkDNS();
                    break;
                case CMD_ZB_ERASE_NVRAM:
                    xTaskCreate(zbEraseNV, "zbEraseNV", 2048, NULL, 5, NULL);
                    break;
                case CMD_ESP_RES:
                    serverWeb.send(HTTP_CODE_OK, contTypeText, result);
                    delay(250);
                    ESP.restart();
                    break;
                case CMD_ADAP_LAN:
                    usbModeSet(XZG);
                    break;
                case CMD_ADAP_USB:
                    usbModeSet(ZIGBEE);
                    break;
                case CMD_ESP_UPD_URL:
                    if (serverWeb.hasArg(argUrl))
                        getEspUpdate(serverWeb.arg(argUrl));
                    else
                    {
                        String link = fetchLatestEspFw();
                        if (link)
                        {
                            getEspUpdate(link);
                        }
                        else
                        {
                            LOGW("%s", String(errLink));
                        }
                    }
                    break;
                case CMD_ZB_CHK_FW:
                    if (zbFwCheck())
                    {
                        serverWeb.send(HTTP_CODE_OK, contTypeText, result);
                    }
                    else
                    {
                        serverWeb.send(HTTP_CODE_INTERNAL_SERVER_ERROR, contTypeText, result);
                    }
                    break;
                case CMD_ZB_CHK_HW:
                    zbHwCheck();
                    break;
                case CMD_ZB_LED_TOG:
                    if (zbLedToggle())
                    {
                        serverWeb.send(HTTP_CODE_OK, contTypeText, result);
                    }
                    else
                    {
                        serverWeb.send(HTTP_CODE_INTERNAL_SERVER_ERROR, contTypeText, result);
                    }
                    break;
                case CMD_ESP_FAC_RES:
                    if (serverWeb.hasArg(argConf))
                        if (serverWeb.arg(argConf).toInt() == 1)
                        {
                            serverWeb.send(HTTP_CODE_OK, contTypeText, result);
                            factoryReset();
                        }
                        else
                        {
                            serverWeb.send(HTTP_CODE_BAD_REQUEST, contTypeText, result);
                        }
                    break;
                case CMD_LED_ACT:
                    if (serverWeb.hasArg(argLed) && serverWeb.hasArg(argAct))
                    {
                        int ledNum = serverWeb.arg(argLed).toInt();
                        int actNum = serverWeb.arg(argAct).toInt();

                        LED_t ledEnum = static_cast<LED_t>(ledNum);
                        LEDMode actEnum = static_cast<LEDMode>(actNum);

                        if (static_cast<int>(ledEnum) == ledNum && static_cast<int>(actEnum) == actNum)
                        {
                            String tag = "API";
                            serverWeb.send(HTTP_CODE_OK, contTypeText, result);
                            if (ledNum == MODE_LED)
                            {
                                LOGD("%s led %d", ledControl.modeLED.name, actNum);
                                ledControl.modeLED.mode = static_cast<LEDMode>(actNum);
                            }
                            else if (ledNum == POWER_LED)
                            {
                                LOGD("%s led %d", ledControl.powerLED.name, actNum);
                                ledControl.powerLED.mode = static_cast<LEDMode>(actNum);
                            }
                        }
                        else
                        {
                            serverWeb.send(HTTP_CODE_BAD_REQUEST, contTypeText, result);
                        }
                    }
                    else
                    {
                        serverWeb.send(HTTP_CODE_BAD_REQUEST, contTypeText, result);
                    }
                    break;
                case CMD_ZB_FLASH:
                    if (serverWeb.hasArg(argUrl))
                        flashZbUrl(serverWeb.arg(argUrl));
                    else
                    {
                        String link = fetchLatestZbFw();
                        if (link)
                        {
                            flashZbUrl(link);
                        }
                        else
                        {
                            LOGW("%s", String(errLink));
                        }
                    }
                    break;
                case CMD_BRD_NAME:
                    if (serverWeb.hasArg("board"))
                    {
                        String brdName = serverWeb.arg("board");
                        brdName.toCharArray(hwConfig.board, sizeof(hwConfig.board));

                        File configFile = LittleFS.open(configFileHw, FILE_READ);
                        if (!configFile)
                        {
                            Serial.println("Failed to open config file for reading");
                            return;
                        }

                        DynamicJsonDocument config(1024);
                        DeserializationError error = deserializeJson(config, configFile);
                        if (error)
                        {
                            Serial.println("Failed to parse config file");
                            configFile.close();
                            return;
                        }

                        configFile.close();
                        config["board"] = hwConfig.board;

                        writeDefaultConfig(configFileHw, config);
                        serverWeb.send(HTTP_CODE_OK, contTypeText, result);
                    }
                    else
                    {
                        serverWeb.send(HTTP_CODE_BAD_REQUEST, contTypeText, result);
                    }
                    break;
                default:
                    serverWeb.send(HTTP_CODE_BAD_REQUEST, contTypeText, result);
                    break;
                }
                serverWeb.send(HTTP_CODE_OK, contTypeText, result);
            }
        }
        break;
        case API_WIFICONNECTSTAT:
        {
            String result;
            StaticJsonDocument<70> doc;
            const char *connected = "connected";
            if (WiFi.status() == WL_CONNECTED)
            {
                doc[connected] = true;
                doc["ip"] = WiFi.localIP().toString();
            }
            else
            {
                doc[connected] = false;
            }
            serializeJson(doc, result);
            serverWeb.send(HTTP_CODE_OK, contTypeJson, result);
        }
        break;
        case API_GET_FILE:
        {
            String result = wrongArgs;

            if (serverWeb.hasArg(argFilename))
            {
                String filename = "/" + serverWeb.arg(argFilename);
                File file = LittleFS.open(filename, "r");
                if (!file)
                    return;
                result = "";
                while (file.available() && result.length() < 500)
                {
                    result += (char)file.read();
                }
                file.close();
            }
            serverWeb.send(HTTP_CODE_OK, contTypeText, result);
        }
        break;
        case API_DEL_FILE:
        {
            String result = wrongArgs;

            if (serverWeb.hasArg(argFilename))
            {
                String filename = "/" + serverWeb.arg(argFilename);
                LOGW("Remove file %s", filename.c_str());
                LittleFS.remove(filename);
            }
            serverWeb.send(HTTP_CODE_OK, contTypeText, result);
        }
        break;
        case API_GET_PARAM:
        {
            String resp = wrongArgs;
            if (serverWeb.hasArg(param))
            {
                if (serverWeb.arg(param) == "refreshLogs")
                {
                    resp = (String)systemCfg.refreshLogs;
                }
                else if (serverWeb.arg(param) == "update_root")
                {
                    resp = getRootData(true);
                }
                else if (serverWeb.arg(param) == "coordMode")
                {
                    if (wifiWebSetupInProgress)
                    {
                        resp = "1";
                    }
                    else
                    {
                        resp = (String)systemCfg.workMode;
                    }
                }
                else if (serverWeb.arg(param) == "zbFwVer")
                {
                    resp = String(CCTool.chip.fwRev);
                }
                else if (serverWeb.arg(param) == "zbHwVer")
                {
                    resp = String(CCTool.chip.hwRev);
                }
                else if (serverWeb.arg(param) == "espVer")
                {
                    resp = VERSION;
                }
                else if (serverWeb.arg(param) == "wifiEnable")
                {
                    resp = networkCfg.wifiEnable;
                }
                else if (serverWeb.arg(param) == "all")
                {
                    resp = makeJsonConfig(&networkCfg, &vpnCfg, &mqttCfg, &systemCfg);
                }
                else if (serverWeb.arg(param) == "vars")
                {
                    resp = makeJsonConfig(NULL, NULL, NULL, NULL, &vars);
                }
                else if (serverWeb.arg(param) == "root")
                {
                    resp = getRootData();
                }
            }
            serverWeb.send(HTTP_CODE_OK, contTypeText, resp);
        }
        break;
        case API_STARTWIFISCAN:
            if (WiFi.getMode() == WIFI_OFF)
            { // enable wifi for scan
                WiFi.mode(WIFI_STA);
            }
            // } else if (WiFi.getMode() == WIFI_AP) {  // enable sta for scan
            //     WiFi.mode(WIFI_AP_STA);
            // }
            WiFi.scanNetworks(true);
            serverWeb.send(HTTP_CODE_OK, contTypeTextHtml, ok);
            break;
        case API_WIFISCANSTATUS:
        {
            static uint8_t timeout = 0;
            DynamicJsonDocument doc(1024);
            String result = "";
            int16_t scanRes = WiFi.scanComplete();
            const char *scanDone = "scanDone";
            doc[scanDone] = false;
            if (scanRes == -2)
            {
                WiFi.scanNetworks(true);
            }
            else if (scanRes > 0)
            {
                doc[scanDone] = true;
                JsonArray wifi = doc.createNestedArray("wifi");
                for (int i = 0; i < scanRes; ++i)
                {
                    JsonObject wifi_0 = wifi.createNestedObject();
                    wifi_0["ssid"] = WiFi.SSID(i);
                    wifi_0["rssi"] = WiFi.RSSI(i);
                    wifi_0["channel"] = WiFi.channel(i);
                    wifi_0["secure"] = WiFi.encryptionType(i);
                }
                WiFi.scanDelete();
            }
            if (timeout < 10)
            {
                timeout++;
            }
            else
            {
                doc[scanDone] = true;
                WiFi.scanDelete();
                timeout = 0;
            }
            serializeJson(doc, result);
            serverWeb.send(HTTP_CODE_OK, contTypeJson, result);
            break;
        }
        case API_GET_PAGE:
            if (!serverWeb.arg(page).length() > 0)
            {
                LOGW("wrong arg 'page' %s", serverWeb.argName(1));
                serverWeb.send(500, contTypeText, wrongArgs);
                return;
            }
            switch (serverWeb.arg(page).toInt())
            {
            case API_PAGE_ROOT:
                handleRoot();
                sendGzip(contTypeTextHtml, PAGE_ROOT_html_gz, PAGE_ROOT_html_gz_len);
                break;
            case API_PAGE_GENERAL:
                handleGeneral();
                sendGzip(contTypeTextHtml, PAGE_GENERAL_html_gz, PAGE_GENERAL_html_gz_len);
                break;
            case API_PAGE_NETWORK:
                handleNetwork();
                sendGzip(contTypeTextHtml, PAGE_NETWORK_html_gz, PAGE_NETWORK_html_gz_len);
                break;
            case API_PAGE_ZIGBEE:
                handleSerial();
                sendGzip(contTypeTextHtml, PAGE_ZIGBEE_html_gz, PAGE_ZIGBEE_html_gz_len);
                break;
            case API_PAGE_SECURITY:
                handleSecurity();
                sendGzip(contTypeTextHtml, PAGE_SECURITY_html_gz, PAGE_SECURITY_html_gz_len);
                break;
            case API_PAGE_TOOLS:
                handleTools();
                sendGzip(contTypeTextHtml, PAGE_TOOLS_html_gz, PAGE_TOOLS_html_gz_len);
                break;
            case API_PAGE_ABOUT:
                // handleAbout();
                sendGzip(contTypeTextHtml, PAGE_ABOUT_html_gz, PAGE_ABOUT_html_gz_len);
                break;
            case API_PAGE_MQTT:
                handleMqtt();
                sendGzip(contTypeTextHtml, PAGE_MQTT_html_gz, PAGE_MQTT_html_gz_len);
                break;
            case API_PAGE_VPN:
                handleVpn();
                sendGzip(contTypeTextHtml, PAGE_VPN_html_gz, PAGE_VPN_html_gz_len);
                break;
            default:
                break;
            }
            break;
        case API_GET_FILELIST:
        {
            String fileList = "";
            DynamicJsonDocument doc(512);
            JsonArray files = doc.createNestedArray("files");
            File root = LittleFS.open("/");
            File file = root.openNextFile();
            while (file)
            {
                JsonObject jsonfile = files.createNestedObject();
                jsonfile["filename"] = String(file.name());
                jsonfile["size"] = file.size();
                file = root.openNextFile();
            }
            root = LittleFS.open("/config/");
            file = root.openNextFile();
            while (file)
            {
                JsonObject jsonfile = files.createNestedObject();
                jsonfile["filename"] = String("/config/" + String(file.name()));
                jsonfile["size"] = file.size();
                file = root.openNextFile();
            }
            serializeJson(doc, fileList);
            serverWeb.send(HTTP_CODE_OK, contTypeJson, fileList);
            break;
        }

        default:
            LOGW("switch (action) err");
            break;
        }
    }
}

void handleSaveParams()
{
    if (!is_authenticated())
        return;
    updateConfiguration(serverWeb, systemCfg, networkCfg, vpnCfg, mqttCfg);
}

void printEachKeyValuePair(const String &jsonString)
{
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error)
    {

        return;
    }

    const uint8_t eventLen = 100;

    for (JsonPair kv : doc.as<JsonObject>())
    {
        DynamicJsonDocument pairDoc(256);
        pairDoc[kv.key().c_str()] = kv.value();

        String output;
        serializeJson(pairDoc, output);

        sendEvent("root_update", eventLen, String(output));
    }
    sendEvent("root_update", eventLen, String("finish"));
}

void updateWebTask(void *parameter)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    const uint8_t eventLen = 100;
    while (1)
    {
        String root_data = getRootData(true);
        printEachKeyValuePair(root_data);
        root_data = String(); // free memory
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(systemCfg.refreshLogs * 1000));
    }
}

void handleLoginGet()
{
    if (!is_authenticated())
    {
        // LOGD("handleLoginGet !is_authenticated");
        sendGzip(contTypeTextHtml, PAGE_LOGIN_html_gz, PAGE_LOGIN_html_gz_len);
    }
    else
    {
        // LOGD("handleLoginGet else");
        serverWeb.sendHeader("Location", "/");
        serverWeb.sendHeader("Cache-Control", "no-cache");
        serverWeb.send(301);
        // sendGzip(contTypeTextHtml, PAGE_LOADER_html_gz, PAGE_LOADER_html_gz_len);
    }
}
void handleLoginPost()
{
    // Serial.println("Handle Login");
    // String msg;

    /*if (serverWeb.hasHeader("Cookie"))
    {
        // Print cookies
        Serial.print("Found cookie: ");
        String cookie = serverWeb.header("Cookie");
        Serial.println(cookie);
    }*/

    if (serverWeb.hasArg("username") && serverWeb.hasArg("password"))
    {
        // Serial.print("Found parameter: ");

        if (serverWeb.arg("username") == String(systemCfg.webUser) && serverWeb.arg("password") == String(systemCfg.webPass))
        {
            serverWeb.sendHeader("Location", "/");
            serverWeb.sendHeader("Cache-Control", "no-cache");

            String token = sha1(String(systemCfg.webUser) + ":" + String(systemCfg.webPass) + ":" + serverWeb.client().remoteIP().toString());
            serverWeb.sendHeader("Set-Cookie", "XZG_UID=" + token);

            serverWeb.send(301);
            // Serial.println("Log in Successful");
            return;
        }
        // msg = ;
        //  Serial.println("Log in Failed");
        redirectLogin(2);
        return;
    }
}

void handleLogout()
{
    // Serial.println("Disconnection");
    serverWeb.sendHeader("Set-Cookie", "XZG_UID=0");
    serverWeb.sendHeader("Authentication", "fail");
    redirectLogin(3);

    // serverWeb.send_P(401, contTypeTextHtml, (const char *)PAGE_LOGOUT_html_gz, PAGE_LOGOUT_html_gz_len); });*/

    return;
}
// Check if header is present and correct
bool is_authenticated()
{
    if (systemCfg.webAuth)
    {
        if (serverWeb.hasHeader("Cookie"))
        {
            String cookie = serverWeb.header("Cookie");
            String token = sha1(String(systemCfg.webUser) + ":" + String(systemCfg.webPass) + ":" + serverWeb.client().remoteIP().toString());

            if (cookie.indexOf("XZG_UID=" + token) != -1)
            {
                // LOGD("Successful");
                serverWeb.sendHeader("Authentication", "ok");
                return true;
            }
        }
        // LOGD("Failed");
        serverWeb.sendHeader("Authentication", "fail");
        return false;
    }
    else
    {
        return true;
    }
}

void handleGeneral()
{
    DynamicJsonDocument doc(1024);
    String result;

    doc[hwBtnIsKey] = vars.hwBtnIs;
    // doc[hwUartSelIsKey] = vars.hwUartSelIs;
    doc[hwLedPwrIsKey] = vars.hwLedPwrIs;
    doc[hwLedUsbIsKey] = vars.hwLedUsbIs;

    /*switch (systemCfg.workMode)
    {
    case WORK_MODE_USB:
        doc["checkedUsbMode"] = checked;
        break;
    case WORK_MODE_NETWORK:
        doc["checkedLanMode"] = checked;
        break;
    default:
        break;
    }

    if (systemCfg.keepWeb)
    {
        doc[keepWebKey] = checked;
    }*/

    if (systemCfg.disableLedPwr)
    {
        doc["checkedDisableLedPwr"] = checked;
    }
    if (systemCfg.disableLedUSB)
    {
        doc["checkedDisableLedUSB"] = checked;
    }

    doc[hostnameKey] = systemCfg.hostname;
    doc[refreshLogsKey] = systemCfg.refreshLogs;

    if (systemCfg.timeZone)
    {
        doc[timeZoneNameKey] = systemCfg.timeZone;
    }
    doc[ntpServ1Key] = systemCfg.ntpServ1; //.toString();
    doc[ntpServ2Key] = systemCfg.ntpServ2; //.toString();

    doc[nmStartHourKey] = systemCfg.nmStart;
    doc[nmEndHourKey] = systemCfg.nmEnd;
    if (systemCfg.nmEnable)
    {
        doc[nmEnableKey] = checked;
    }

    if (systemCfg.updAutoInst)
    {
        doc[updAutoInstKey] = checked;
    }
    doc[updCheckTimeKey] = systemCfg.updCheckTime;
    doc[updCheckDayKey] = systemCfg.updCheckDay;

    serializeJson(doc, result);
    serverWeb.sendHeader(respHeaderName, result);

    DynamicJsonDocument zones(8000);
    String results;

    JsonArray zonesArray = zones.to<JsonArray>();
    for (int i = 0; i < timeZoneCount; i++)
    {
        zonesArray.add(timeZones[i].zone);
    }

    // size_t usedMemory = zones.memoryUsage();
    // LOGD("Zones used: %s bytes", String(usedMemory));

    serializeJson(zones, results);
    serverWeb.sendHeader(respTimeZonesName, results);
}

void handleSecurity()
{
    String result;
    DynamicJsonDocument doc(1024);

    if (systemCfg.disableWeb)
    {
        doc[disableWebKey] = checked;
    }

    if (systemCfg.webAuth)
    {
        doc[webAuthKey] = checked;
    }
    doc[webUserKey] = (String)systemCfg.webUser;
    doc[webPassKey] = (String)systemCfg.webPass;
    if (systemCfg.fwEnabled)
    {
        doc[fwEnabledKey] = checked;
    }
    doc[fwIpKey] = systemCfg.fwIp; //.toString();

    serializeJson(doc, result);
    serverWeb.sendHeader(respHeaderName, result);
}

void handleNetwork()
{
    String result;
    DynamicJsonDocument doc(1024);

    // doc["pageName"] = "Config WiFi";

    if (networkCfg.ethEnable)
    {
        doc[ethEnblKey] = checked;
    }
    if (networkCfg.ethDhcp)
    {
        doc[ethDhcpKey] = checked;
    }
    doc[ethIpKey] = networkCfg.ethIp;
    doc[ethMaskKey] = networkCfg.ethMask;
    doc[ethGateKey] = networkCfg.ethGate;

    doc[ethDns1Key] = networkCfg.ethDns1;
    doc[ethDns2Key] = networkCfg.ethDns2;

    if (networkCfg.wifiEnable)
    {
        doc[wifiEnblKey] = checked;
    }
    doc[wifiSsidKey] = String(networkCfg.wifiSsid);
    doc[wifiPassKey] = String(networkCfg.wifiPass);
    if (networkCfg.wifiDhcp)
    {
        doc[wifiDhcpKey] = checked;
    }
    doc[wifiIpKey] = networkCfg.wifiIp;
    doc[wifiMaskKey] = networkCfg.wifiMask;
    doc[wifiGateKey] = networkCfg.wifiGate;
    doc[wifiDns1Key] = networkCfg.wifiDns1;
    doc[wifiDns2Key] = networkCfg.wifiDns2;

    switch (networkCfg.wifiMode)
    {
    case WIFI_PROTOCOL_11B:
        doc["1"] = checked;
        break;
    case WIFI_PROTOCOL_11G:
        doc["2"] = checked;
        break;
    case WIFI_PROTOCOL_11N:
        doc["4"] = checked;
        break;
    case WIFI_PROTOCOL_LR:
        doc["8"] = checked;
        break;
    default:
        break;
    }

    switch (networkCfg.wifiPower)
    {
    case WIFI_POWER_19_5dBm:
        doc["78"] = checked;
        break;
    case WIFI_POWER_19dBm:
        doc["76"] = checked;
        break;
    case WIFI_POWER_18_5dBm:
        doc["74"] = checked;
        break;
    case WIFI_POWER_17dBm:
        doc["68"] = checked;
        break;
    case WIFI_POWER_15dBm:
        doc["60"] = checked;
        break;
    case WIFI_POWER_13dBm:
        doc["52"] = checked;
        break;
    case WIFI_POWER_11dBm:
        doc["44"] = checked;
        break;
    case WIFI_POWER_8_5dBm:
        doc["34"] = checked;
        break;
    case WIFI_POWER_7dBm:
        doc["28"] = checked;
        break;
    case WIFI_POWER_5dBm:
        doc["20"] = checked;
        break;
    case WIFI_POWER_2dBm:
        doc["8"] = checked;
        break;
    default:
        break;
    }

    if (hwConfig.eth.mdcPin == -1 || hwConfig.eth.mdiPin == -1)
    {
        doc["no_eth"] = 1;
    }

    serializeJson(doc, result);
    serverWeb.sendHeader(respHeaderName, result);
}

void handleSerial()
{
    String result;
    DynamicJsonDocument doc(1024);

    switch (systemCfg.workMode)
    {
    case WORK_MODE_USB:
        doc["usbMode"] = checked;
        break;
    case WORK_MODE_NETWORK:
        doc["lanMode"] = checked;
        break;
    default:
        break;
    }

    switch (systemCfg.serialSpeed)
    {
    case 9600:
        doc["9600"] = checked;
        break;
    case 19200:
        doc["19200"] = checked;
        break;
    case 38400:
        doc["38400"] = checked;
        break;
    case 57600:
        doc["57600"] = checked;
        break;
    case 115200:
        doc["115200"] = checked;
        break;
    case 230400:
        doc["230400"] = checked;
        break;
    case 460800:
        doc["460800"] = checked;
        break;
    default:
        doc["115200"] = checked;
        break;
    }

    doc[socketPortKey] = String(systemCfg.socketPort);

    doc[zbRoleKey] = systemCfg.zbRole;

    serializeJson(doc, result);
    serverWeb.sendHeader(respHeaderName, result);
}

void handleMqtt()
{
    String result;
    DynamicJsonDocument doc(1024);

    if (mqttCfg.enable)
    {
        doc["enableMqtt"] = checked;
    }
    doc["serverMqtt"] = mqttCfg.server;
    doc["portMqtt"] = mqttCfg.port;
    doc["userMqtt"] = mqttCfg.user;
    doc["passMqtt"] = mqttCfg.pass;
    doc["topicMqtt"] = mqttCfg.topic;
    doc["intervalMqtt"] = mqttCfg.updateInt;
    doc["mqttReconnect"] = mqttCfg.reconnectInt;

    if (mqttCfg.discovery)
    {
        doc["discoveryMqtt"] = checked;
    }

    serializeJson(doc, result);
    serverWeb.sendHeader(respHeaderName, result);
}

void handleVpn()
{
    String result;
    DynamicJsonDocument doc(1024);

    if (vpnCfg.wgEnable)
    {
        doc[wgEnableKey] = checked;
    }
    doc[wgLocalIPKey] = vpnCfg.wgLocalIP.toString();
    doc[wgLocalSubnetKey] = vpnCfg.wgLocalSubnet.toString();
    doc[wgLocalPortKey] = vpnCfg.wgLocalPort;
    doc[wgLocalGatewayKey] = vpnCfg.wgLocalGateway.toString();
    doc[wgLocalPrivKeyKey] = vpnCfg.wgLocalPrivKey;
    doc[wgEndAddrKey] = vpnCfg.wgEndAddr;
    doc[wgEndPubKeyKey] = vpnCfg.wgEndPubKey;
    doc[wgEndPortKey] = vpnCfg.wgEndPort;
    doc[wgAllowedIPKey] = vpnCfg.wgAllowedIP.toString();
    doc[wgAllowedMaskKey] = vpnCfg.wgAllowedMask.toString();
    if (vpnCfg.wgMakeDefault)
    {
        doc[wgMakeDefaultKey] = checked;
    }
    doc[wgPreSharedKeyKey] = vpnCfg.wgPreSharedKey;

    if (vpnCfg.hnEnable)
    {
        doc[hnEnableKey] = checked;
    }
    doc[hnJoinCodeKey] = vpnCfg.hnJoinCode;
    doc[hnHostNameKey] = vpnCfg.hnHostName;
    doc[hnDashUrlKey] = vpnCfg.hnDashUrl;

    serializeJson(doc, result);
    serverWeb.sendHeader(respHeaderName, result);
}

String getRootData(bool update)
{
    String tag = "root_data";
    DynamicJsonDocument doc(2048);

    String readableTime;
    getReadableTime(readableTime, vars.socketTime);
    const char *connectedSocketStatus = "connectedSocketStatus";
    const char *connectedSocket = "connectedSocket";
    const char *noConn = "noConn";

    doc[connectedSocketStatus] = vars.connectedClients;
    doc[connectedSocket] = vars.socketTime;
    doc["localTime"] = getTime();

    if (!update)
    {
        char verArr[25];
        const char *env = STRINGIFY(BUILD_ENV_NAME);

        if (strcasestr(env, "debug") != NULL)
        {
            sprintf(verArr, "%s (%s)", VERSION, env);
        }
        else
        {
            sprintf(verArr, "%s", VERSION);
        }

        doc["VERSION"] = String(verArr);

        const char *operationalMode = "operationalMode";
        doc[operationalMode] = systemCfg.workMode;

        doc[espUpdAvailKey] = vars.updateEspAvail;
        doc[zbUpdAvailKey] = vars.updateZbAvail;
    }

    // ETHERNET TAB
    const char *ethConn = "ethConn";
    const char *ethMac = "ethMac";
    const char *ethSpd = "ethSpd";
    const char *ethDns = "ethDns";

    if (networkCfg.ethEnable)
    {
        if (!update)
        {
            doc[ethMac] = ETH.macAddress();
        }
        doc[ethConn] = vars.connectedEther ? 1 : 0;
        doc[ethDhcpKey] = networkCfg.ethDhcp ? 1 : 0;
        if (vars.connectedEther)
        {
            doc[ethSpd] = ETH.linkSpeed();
            doc[ethIpKey] = ETH.localIP().toString();
            doc[ethMaskKey] = ETH.subnetMask().toString();
            doc[ethGateKey] = ETH.gatewayIP().toString();
            doc[ethDns] = vars.savedEthDNS.toString(); // ETH.dnsIP().toString();
        }
        else
        {
            doc[ethSpd] = noConn;
            doc[ethIpKey] = networkCfg.ethDhcp ? noConn : ETH.localIP().toString();
            doc[ethMaskKey] = networkCfg.ethDhcp ? noConn : ETH.subnetMask().toString();
            doc[ethGateKey] = networkCfg.ethDhcp ? noConn : ETH.gatewayIP().toString();
            doc[ethDns] = networkCfg.ethDhcp ? noConn : vars.savedEthDNS.toString(); // ETH.dnsIP().toString();
        }
    }

    doc["uptime"] = millis(); // readableTime;

    float CPUtemp = getCPUtemp();
    doc["deviceTemp"] = String(CPUtemp);

    if (vars.oneWireIs)
    {
        float temp = get1wire();
        doc["1wTemp"] = String(temp);
    }

    if (!update)
    {
        doc["hwRev"] = hwConfig.board;
        doc["espModel"] = String(ESP.getChipModel());
        doc["espCores"] = ESP.getChipCores();
        doc["espFreq"] = ESP.getCpuFreqMHz();

        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);

        const char *espFlashType = "espFlashType";
        if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
        {
            doc[espFlashType] = 1;
        }
        else
        {
            doc[espFlashType] = 2;
        }

        doc["espFlashSize"] = ESP.getFlashChipSize() / (1024 * 1024);

        // doc["zigbeeFwRev"] = String(CCTool.chip.fwRev);
        if (CCTool.chip.fwRev > 0)
        {
            doc["zigbeeFwRev"] = String(CCTool.chip.fwRev);
        }
        else
        {
            doc["zigbeeFwRev"] = String(systemCfg.zbFw);
            doc["zbFwSaved"] = true;
        }

        doc["zigbeeHwRev"] = CCTool.chip.hwRev;

        doc["zigbeeIeee"] = CCTool.chip.ieee;

        doc["zigbeeFlSize"] = String(CCTool.chip.flashSize / 1024);

        unsigned int totalFs = LittleFS.totalBytes() / 1024;
        unsigned int usedFs = LittleFS.usedBytes() / 1024;

        doc["espFsSize"] = totalFs;
        doc["espFsUsed"] = usedFs;

        doc[zbRoleKey] = systemCfg.zbRole;
        doc["zigbeeFwSaved"] = systemCfg.zbFw;
    }
    int heapSize = ESP.getHeapSize() / 1024;
    int heapFree = ESP.getFreeHeap() / 1024;

    doc["espHeapSize"] = heapSize;
    doc["espHeapUsed"] = heapSize - heapFree;

    int total, used;
    getNvsStats(&total, &used);

    doc["espNvsSize"] = total;
    doc["espNvsUsed"] = used;

    // wifi
    const char *wifiRssi = "wifiRssi";
    const char *wifiConn = "wifiConn";
    const char *wifiMode = "wifiMode";
    const char *wifiDns = "wifiDns";

    if (!update)
    {
        doc["wifiMac"] = WiFi.macAddress();
        String boardStr = hwConfig.board;
        if (boardStr.startsWith("Multi"))
        {
            int underscoreIndex = boardStr.indexOf('_');
            if (underscoreIndex != -1)
            {
                String boardNumber = boardStr.substring(underscoreIndex + 1);
                boardNumber.trim();
                int boardNum = boardNumber.toInt();

                String boardArray[BOARD_CFG_CNT];
                int arrayIndex = 0;

                for (int brdNewIdx = 0; brdNewIdx < BOARD_CFG_CNT; brdNewIdx++)
                {
                    if (brdConfigs[brdNewIdx].ethConfigIndex == brdConfigs[boardNum].ethConfigIndex)
                    {
                        if (brdConfigs[brdNewIdx].zbConfigIndex == brdConfigs[boardNum].zbConfigIndex)
                        {
                            if (brdConfigs[brdNewIdx].mistConfigIndex == brdConfigs[boardNum].mistConfigIndex)
                            {
                                boardArray[arrayIndex++] = brdConfigs[brdNewIdx].board;
                            }
                        }
                    }
                }

                DynamicJsonDocument jsonDoc(512);
                JsonArray jsonArray = jsonDoc.to<JsonArray>();

                for (int i = 0; i < arrayIndex; i++)
                {
                    jsonArray.add(boardArray[i]);
                }

                String jsonString;
                serializeJson(jsonArray, jsonString);

                doc["boardArray"] = jsonString;
            }
        }
    }

    if (networkCfg.wifiEnable)
    {
        doc[wifiMode] = 1; //"Client";
        doc[wifiDhcpKey] = networkCfg.wifiDhcp ? 1 : 0;
        if (WiFi.status() == WL_CONNECTED)
        { // STA connected
            doc[wifiSsidKey] = WiFi.SSID();
            doc[wifiRssi] = WiFi.RSSI();
            doc[wifiConn] = 1;
            doc[wifiIpKey] = WiFi.localIP().toString();
            doc[wifiMaskKey] = WiFi.subnetMask().toString();
            doc[wifiGateKey] = WiFi.gatewayIP().toString();
            doc[wifiDns] = vars.savedWifiDNS.toString(); // WiFi.dnsIP().toString();
        }
        else
        {
            doc[wifiSsidKey] = networkCfg.wifiSsid;
            doc[wifiRssi] = noConn;
            doc[wifiConn] = 0;
            doc[wifiIpKey] = networkCfg.wifiDhcp ? noConn : WiFi.localIP().toString();
            doc[wifiMaskKey] = networkCfg.wifiDhcp ? noConn : WiFi.subnetMask().toString();
            doc[wifiGateKey] = networkCfg.wifiDhcp ? noConn : WiFi.gatewayIP().toString();
            doc[wifiDns] = networkCfg.wifiDhcp ? noConn : vars.savedWifiDNS.toString(); // WiFi.dnsIP().toString();
        }
    }

    if (vars.apStarted)
    { // AP active
        String AP_NameString;

        doc[wifiMode] = 2;
        doc[wifiConn] = 1;
        doc[wifiSsidKey] = vars.deviceId;
        doc[wifiIpKey] = WiFi.localIP().toString(); //"192.168.1.1 (XZG web interface)";
        doc[wifiMaskKey] = "255.255.255.0 (Access point)";
        doc[wifiGateKey] = "192.168.1.1 (this device)";
        doc[wifiDhcpKey] = "On (Access point)";
        doc[wifiMode] = 2;      //"AP";
        doc[wifiRssi] = noConn; //"N/A";
    }

    // MQTT
    if (mqttCfg.enable)
    {
        const char *mqConnect = "mqConnect";
        const char *mqBroker = "mqBroker";

        doc[mqBroker] = mqttCfg.server;

        doc[mqConnect] = vars.mqttConn ? 1 : 0;
    }

    // VPN WireGuard
    if (vpnCfg.wgEnable)
    {
        const char *wgInit = "wgInit";
        const char *wgDeviceAddr = "wgDeviceAddr";
        const char *wgRemoteAddr = "wgRemoteAddr";
        const char *wgConnect = "wgConnect";
        const char *wgRemoteIP = "wgRemoteIp";
        // const char *wgEndPort = "wgEndPort";

        doc[wgInit] = vars.vpnWgInit ? 1 : 0;
        doc[wgDeviceAddr] = vpnCfg.wgLocalIP.toString();
        doc[wgRemoteAddr] = vpnCfg.wgEndAddr;
        // doc[wgEndPort] = vpnCfg.wgEndPort;

        doc[wgConnect] = vars.vpnWgConnect ? 1 : 0;

        doc[wgRemoteIP] = vars.vpnWgPeerIp.toString();
    }
    // VPN Husarnet
    if (vpnCfg.hnEnable)
    {
        const char *hnInit = "hnInit";
        const char *hnHostName = "hnHostName";

        // doc[wgDeviceAddr] = vpnCfg.wgLocalIP.toString();//WgSettings.localAddr;
        doc[hnHostName] = vpnCfg.hnHostName;

        doc[hnInit] = vars.vpnHnInit ? 1 : 0;
    }

    String result;
    serializeJson(doc, result);

    return result;
}

void handleRoot()
{
    String result = getRootData();
    serverWeb.sendHeader(respHeaderName, result);
}

void handleTools()
{
    String result;
    DynamicJsonDocument doc(512);

    doc[hwBtnIsKey] = vars.hwBtnIs;
    // doc[hwUartSelIsKey] = vars.hwUartSelIs;
    doc[hwLedPwrIsKey] = vars.hwLedPwrIs;
    doc[hwLedUsbIsKey] = vars.hwLedUsbIs;
    // doc["hostname"] = systemCfg.hostname;
    // doc["refreshLogs"] = systemCfg.refreshLogs;

    serializeJson(doc, result);
    serverWeb.sendHeader(respHeaderName, result);
}

void handleSavefile()
{
    if (!is_authenticated())
        return;
    if (serverWeb.method() != HTTP_POST)
    {
        serverWeb.send(405, contTypeText, F("Method Not Allowed"));
    }
    else
    {
        String filename = "/" + serverWeb.arg(0);
        String content = serverWeb.arg(1);
        File file = LittleFS.open(filename, "w");
        LOGD("try %s", filename.c_str());

        if (!file)
        {
            LOGW("Failed to open file for reading");
            return;
        }

        int bytesWritten = file.print(content);
        if (bytesWritten > 0)
        {
            LOGD("File was written, %d", bytesWritten);
        }
        else
        {
            LOGW("File write failed");
        }

        file.close();
        serverWeb.sendHeader(F("Location"), F("/tools"));
        serverWeb.send(303);
    }
}

/* ----- Multi-tool support | START -----*/
void handleZigbeeBSL()
{

    zigbeeEnableBSL();
    serverWeb.send(HTTP_CODE_OK, contTypeText, "Zigbee BSL");
}

void handleZigbeeRestart()
{
    zigbeeRestart();
    serverWeb.send(HTTP_CODE_OK, contTypeText, "Zigbee Restart");
}
/* ----- Multi-tool support | END -----*/

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
    LOGI("%s", msg.c_str());
}
/*
void progressNvRamFunc(unsigned int progress, unsigned int total)
{

    const char *tagESP_FW_prgs = "ESP_FW_prgs";
    const uint8_t eventLen = 11;

    float percent = ((float)progress / total) * 100.0;

    sendEvent(tagESP_FW_prgs, eventLen, String(percent));
    // printLogMsg(String(percent));

#ifdef DEBUG
    if (int(percent) % 5 == 0)
    {
        LOGD("Update ESP32 progress: %s of %s | %s%", String(progress), String(total), String(percent));
    }
#endif
};
*/
void progressFunc(unsigned int progress, unsigned int total)
{

    const uint8_t eventLen = 11;

    float percent = ((float)progress / total) * 100.0;

    sendEvent(tagESP_FW_prgs, eventLen, String(percent));
    // printLogMsg(String(percent));

#ifdef DEBUG
    if (int(percent) % 5 == 0)
    {
        LOGD("Update ESP32 progress: %s of %s | %s%", String(progress), String(total), String(percent));
    }
#endif
};

int totalLength;       // total size of firmware
int currentLength = 0; // current size of written firmware

void getEspUpdate(String esp_fw_url)
{
    LOGI("getEspUpdate: %s", esp_fw_url.c_str());

    checkDNS();
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure(); // the magic line, use with caution
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    http.begin(client, esp_fw_url);
    http.addHeader("Content-Type", "application/octet-stream");

    // Get file, just to check if each reachable
    int resp = http.GET();
    LOGD("Response: %s", String(resp));
    // If file is reachable, start downloading
    if (resp == HTTP_CODE_OK)
    {
        // get length of document (is -1 when Server sends no Content-Length header)
        totalLength = http.getSize();
        // transfer to local variable
        int len = totalLength;
        // this is required to start firmware update process
        Update.begin(totalLength);
        Update.onProgress(progressFunc);
        LOGI("FW Size: %s", String(totalLength));
        // create buffer for read
        uint8_t buff[128] = {0};
        // get tcp stream
        WiFiClient *stream = http.getStreamPtr();
        // read all data from server
        LOGI("Updating firmware...");
        while (http.connected() && (len > 0 || len == -1))
        {
            // get available data size
            size_t size = stream->available();
            if (size)
            {
                // read up to 128 byte
                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                // pass to function
                runEspUpdateFirmware(buff, c);
                if (len > 0)
                {
                    len -= c;
                }
            }
        }
    }
    else
    {
        LOGI("Cannot download firmware file.");
    }
    http.end();
}

void runEspUpdateFirmware(uint8_t *data, size_t len)
{
    Update.write(data, len);
    currentLength += len;

    // if current length of written firmware is not equal to total firmware size, repeat
    if (currentLength != totalLength)
        return;
    // only if currentLength == totalLength
    Update.end(true);
    LOGD("Update success. Rebooting...");
    // Restart ESP32 to see changes
    ESP.restart();
}

String fetchLatestEspFw()
{
    checkDNS();
    HTTPClient http;
    http.begin("https://api.github.com/repos/xyzroe/XZG/releases");
    int httpCode = http.GET();

    String browser_download_url = "";

    if (httpCode > 0)
    {
        String payload = http.getString();

        DynamicJsonDocument doc(8192);
        deserializeJson(doc, payload);
        JsonArray releases = doc.as<JsonArray>();

        if (releases.size() > 0 && releases[0]["assets"].size() > 1)
        {
            browser_download_url = releases[0]["assets"][1]["browser_download_url"].as<String>();
            // LOGD("browser_download_url: %s", browser_download_url.c_str());
        }
    }
    else
    {
        LOGD("Error on HTTP request");
    }

    http.end();
    return browser_download_url;
}

String fetchLatestZbFw()
{
    checkDNS();
    HTTPClient http;
    http.begin("https://raw.githubusercontent.com/xyzroe/XZG/zb_fws/ti/manifest.json");
    int httpCode = http.GET();

    String browser_download_url = "";

    if (httpCode > 0)
    {
        String payload = http.getString();

        DynamicJsonDocument doc(8192 * 2);
        deserializeJson(doc, payload);

        size_t usedMemory = doc.memoryUsage();
        LOGD("doc used: %s bytes", String(usedMemory));

        String roleKey;
        if (systemCfg.zbRole == COORDINATOR)
        {
            roleKey = "coordinator";
        }
        else if (systemCfg.zbRole == ROUTER)
        {
            roleKey = "router";
        }
        else if (systemCfg.zbRole == OPENTHREAD)
        {
            roleKey = "thread";
        }

        if (doc.containsKey(roleKey) && doc[roleKey].containsKey(CCTool.chip.hwRev))
        {
            JsonObject roleObj = doc[roleKey][CCTool.chip.hwRev].as<JsonObject>();
            for (JsonPair kv : roleObj)
            {
                JsonObject file = kv.value().as<JsonObject>();
                if (file.containsKey("link"))
                {
                    browser_download_url = file["link"].as<String>();
                    if (file.containsKey("baud"))
                    {
                        browser_download_url = browser_download_url + "?b=" + file["baud"].as<int>();
                    }
                    break;
                }
            }
            // LOGD("browser_download_url: %s", browser_download_url.c_str());
        }
    }
    else
    {
        LOGD("Error on HTTP request");
    }

    http.end();
    return browser_download_url;
}

String extractVersionFromURL(String url)
{
    int startPos = url.lastIndexOf('_') + 1;
    int endPos = url.indexOf(".ota.bin");
    if (endPos == -1)
    {
        endPos = url.indexOf(".bin");
    }

    if (startPos != -1 && endPos != -1)
    {
        return url.substring(startPos, endPos);
    }
    return "";
}
