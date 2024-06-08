#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ETH.h>
#include <HTTPClient.h>

// #include <esp_task_wdt.h>
#include <CCTools.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "zb.h"
#include "const/keys.h"

extern struct SysVarsStruct vars;
extern struct ThisConfigStruct hwConfig;
extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct VpnConfigStruct vpnCfg;
extern struct MqttConfigStruct mqttCfg;

extern LEDControl ledControl;

extern const char *tempFile;

size_t lastSize = 0;

String tag_ZB = "[ZB]";

extern CCTools CCTool;

bool zbFwCheck()
{
    const int maxAttempts = 3;
    for (int attempt = 0; attempt < maxAttempts; attempt++)
    {
        if (CCTool.checkFirmwareVersion())
        {
            printLogMsg(tag_ZB + " FW: " + String(CCTool.chip.fwRev));
            return true;
        }
        else
        {
            CCTool.restart();
            int val = attempt + 1;
            LOGD("Try: %d", val);
            delay(500 * (val * val));
        }
    }
    printLogMsg(tag_ZB + " FW: Unknown! Check serial speed!");
    return false;
}

void zbHwCheck()
{
    ledControl.modeLED.mode = LED_BLINK_1Hz;

    if (CCTool.detectChipInfo())
    {
        printLogMsg(tag_ZB + " Chip: " + CCTool.chip.hwRev);
        printLogMsg(tag_ZB + " IEEE: " + CCTool.chip.ieee);
        LOGI("modeCfg %s", String((CCTool.chip.modeCfg), HEX));
        LOGI("bslCfg %s", String((CCTool.chip.bslCfg), HEX));
        printLogMsg(tag_ZB + " Flash size: " + String(CCTool.chip.flashSize / 1024) + " KB");

        vars.hwZigbeeIs = true;
        ledControl.modeLED.mode = LED_OFF;
    }
    else
    {
        printLogMsg(tag_ZB + " No Zigbee chip!");
        vars.hwZigbeeIs = false;
        ledControl.modeLED.mode = LED_BLINK_3Hz;
    }
    CCTool.restart();
}

bool zbLedToggle()
{
    if (CCTool.ledToggle())
    {
        if (CCTool.ledState == 1)
        {
            printLogMsg("[ZB] LED toggle ON");
            // vars.zbLedState = 1;
        }
        else
        {
            printLogMsg("[ZB] LED toggle OFF");
            // vars.zbLedState = 0;
        }
        return true;
    }
    else
    {
        printLogMsg("[ZB] LED toggle ERROR");
        return false;
    }
}

bool zigbeeErase()
{
    if (CCTool.eraseFlash())
    {
        LOGD("magic");
        return true;
    }
    return false;
}
void nvPrgs(const String &inputMsg)
{

    const uint8_t eventLen = 30;
    String msg = inputMsg;
    if (msg.length() > 25)
    {
        msg = msg.substring(0, 25);
    }
    sendEvent(tagZB_NV_prgs, eventLen, msg);
    LOGD("%s", msg.c_str());
}

void zbEraseNV(void *pvParameters)
{
    CCTool.nvram_reset(nvPrgs);
    logClear();
    printLogMsg("NVRAM erase finish! Restart CC2652!");
    vTaskDelete(NULL);
}

void flashZbUrl(String url)
{
    // zbFwCheck();
    ledControl.modeLED.mode = LED_BLINK_3Hz;

    checkDNS();
    delay(250);

    Serial2.updateBaudRate(500000);
    float last_percent = 0;

    const uint8_t eventLen = 11;

    auto progressShow = [last_percent](float percent) mutable
    {
        if ((percent - last_percent) > 1 || percent < 0.1 || percent == 100)
        {
            // char buffer[100];
            // snprintf(buffer, sizeof(buffer), "Flash progress: %.2f%%", percent);
            // printLogMsg(String(buffer));
            LOGI("%.2f%%", percent);
            sendEvent(tagZB_FW_prgs, eventLen, String(percent));
            last_percent = percent;
        }
    };

    printLogMsg("Start Zigbee flashing");
    sendEvent(tagZB_FW_info, eventLen, String("start"));

    // https://raw.githubusercontent.com/xyzroe/XZG/zb_fws/ti/coordinator/CC1352P7_coordinator_20240316.bin
    //  CCTool.enterBSL();
    int key = url.indexOf("?b=");

    String clear_url = url.substring(0, key);

    // printLogMsg("Clear from " + clear_url);
    String baud_str = url.substring(key + 3, url.length());
    // printLogMsg("Baud " + baud_str);
    systemCfg.serialSpeed = baud_str.toInt();

    printLogMsg("ZB flash " + clear_url + " @ " + systemCfg.serialSpeed);

    sendEvent(tagZB_FW_file, eventLen, String(clear_url));

    if (eraseWriteZbUrl(clear_url.c_str(), progressShow, CCTool))
    {
        sendEvent(tagZB_FW_info, eventLen, String("finish"));
        printLogMsg("Flashed successfully");
        Serial2.updateBaudRate(systemCfg.serialSpeed);

        int lineIndex = clear_url.lastIndexOf("_");
        int binIndex = clear_url.lastIndexOf(".bin");
        int lastSlashIndex = clear_url.lastIndexOf("/");

        if (lineIndex > -1 && binIndex > -1 && lastSlashIndex > -1)
        {
            String zbFw = clear_url.substring(lineIndex + 1, binIndex);
            // LOGI("1 %s", zbFw);

            strncpy(systemCfg.zbFw, zbFw.c_str(), sizeof(systemCfg.zbFw) - 1);
            zbFw = "";

            int i = 0;

            int preLastSlash = -1;
            // LOGI("2 %s", String(url.c_str()));

            while (clear_url.indexOf("/", i) > -1 && i < lastSlashIndex)
            {
                int result = clear_url.indexOf("/", i);
                // LOGI("r %d", result);
                if (result > -1)
                {
                    i = result + 1;
                    if (result < lastSlashIndex)
                    {
                        preLastSlash = result;
                        // LOGI("pl %d", preLastSlash);
                    }
                }
                // LOGI("l %d", lastSlashIndex);
                // delay(500);
            }

            // LOGD("%s %s", String(preLastSlash), String(lastSlashIndex));
            String zbRole = clear_url.substring(preLastSlash + 1, lastSlashIndex);
            LOGI("%s", zbRole);

            if (zbRole.indexOf("coordinator") > -1)
            {
                systemCfg.zbRole = COORDINATOR;
            }
            else if (zbRole.indexOf("router") > -1)
            {
                systemCfg.zbRole = ROUTER;
            }
            else if (zbRole.indexOf("tread") > -1)
            {
                systemCfg.zbRole = OPENTHREAD;
            }
            else
            {
                systemCfg.zbRole = UNDEFINED;
            }
            zbRole = "";

            saveSystemConfig(systemCfg);
        }
        else
        {
            LOGW("URL error");
        }
        if (systemCfg.zbRole == COORDINATOR)
        {
            zbFwCheck();
            zbLedToggle();
            delay(1000);
            zbLedToggle();
        }
        sendEvent(tagZB_FW_file, eventLen, String(systemCfg.zbFw));
        delay(500);
        ESP.restart();
    }
    else
    {
        Serial2.updateBaudRate(systemCfg.serialSpeed);
        printLogMsg("Failed to flash Zigbee");
        sendEvent(tagZB_FW_err, eventLen, String("Failed!"));
    }
    ledControl.modeLED.mode = LED_OFF;
}

/*void printBufferAsHex(const byte *buffer, size_t length)
{
    const char *TAG = "BufferHex";
    char hexStr[CCTool.TRANSFER_SIZE + 10];
    std::string hexOutput;

    for (size_t i = 0; i < length; ++i)
    {
        if (buffer[i] != 255)
        {
            sprintf(hexStr, "%02X ", buffer[i]);
            hexOutput += hexStr;
        }
    }

    LOGD("Buffer content:\n%s", hexOutput.c_str());
}*/

bool eraseWriteZbUrl(const char *url, std::function<void(float)> progressShow, CCTools &CCTool)
{
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int loadedSize = 0;
    int totalSize = 0;
    int maxRetries = 7;
    int retryCount = 0;
    int retryDelay = 500;
    bool isSuccess = false;

    while (retryCount < maxRetries && !isSuccess)
    {
        if (loadedSize == 0)
        {
            CCTool.eraseFlash();
            sendEvent("ZB_FW_info", 11, String("erase"));
            printLogMsg("Erase completed!");
        }

        http.begin(client, url);
        http.addHeader("Content-Type", "application/octet-stream");

        if (loadedSize > 0)
        {
            http.addHeader("Range", "bytes=" + String(loadedSize) + "-");
        }

        int httpCode = http.GET();

        if (httpCode != HTTP_CODE_OK && httpCode != HTTP_CODE_PARTIAL_CONTENT)
        {
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "Failed to download file, HTTP code: %d\n", httpCode);
            printLogMsg(buffer);

            http.end();
            retryCount++;
            delay(retryDelay);
            continue;
        }

        if (totalSize == 0)
        {
            totalSize = http.getSize();
        }

        if (loadedSize == 0)
        {
            if (!CCTool.beginFlash(BEGIN_ZB_ADDR, totalSize))
            {
                http.end();
                printLogMsg("Error initializing flash process");
                continue;
            }
            printLogMsg("Begin flash");
        }

        byte buffer[CCTool.TRANSFER_SIZE];
        WiFiClient *stream = http.getStreamPtr();

        while (http.connected() && loadedSize < totalSize)
        {
            size_t size = stream->available();
            if (size > 0)
            {
                int c = stream->readBytes(buffer, std::min(size, sizeof(buffer)));
                if (!CCTool.processFlash(buffer, c))
                {
                    loadedSize = 0;
                    retryCount++;
                    delay(retryDelay);
                    break;
                }
                loadedSize += c;
                float percent = static_cast<float>(loadedSize) / totalSize * 100.0f;
                progressShow(percent);
            }
            else
            {
                delay(1); // Yield to the WiFi stack
            }
        }

        http.end();

        if (loadedSize >= totalSize)
        {
            isSuccess = true;
        }
    }

    CCTool.restart();
    return isSuccess;
}

#include <FS.h>
#include <LittleFS.h>

bool eraseWriteZbFile(const char *filePath, std::function<void(float)> progressShow, CCTools &CCTool)
{
    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "Failed to open file: %s\n", filePath);
        printLogMsg(buffer);
        return false;
    }

    CCTool.eraseFlash();
    printLogMsg("Erase completed!");

    int totalSize = file.size();

    if (!CCTool.beginFlash(BEGIN_ZB_ADDR, totalSize))
    {
        file.close();
        return false;
    }

    byte buffer[CCTool.TRANSFER_SIZE];
    int loadedSize = 0;

    while (file.available() && loadedSize < totalSize)
    {
        size_t size = file.available();
        int c = file.readBytes(reinterpret_cast<char *>(buffer), std::min(size, sizeof(buffer)));
        // printBufferAsHex(buffer, c);
        CCTool.processFlash(buffer, c);
        loadedSize += c;
        float percent = static_cast<float>(loadedSize) / totalSize * 100.0f;
        progressShow(percent);
        delay(1); // Yield to allow other processes
    }

    file.close();
    CCTool.restart();
    return true;
}
