#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ETH.h>
#include <HTTPClient.h>

// #include <WiFiClientSecure.h>
// #include <esp_task_wdt.h>
#include <CCTools.h>

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "zb.h"
#include "mqtt.h"
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
        LOGD("Try: %d", attempt+1);
        delay(500 * (attempt * 2));
        if (CCTool.checkFirmwareVersion())
        {
            printLogMsg(tag_ZB + " FW: " + String(CCTool.chip.fwRev));
            if (systemCfg.zbRole == UNDEFINED)
            {
                systemCfg.zbRole = COORDINATOR;
                saveSystemConfig(systemCfg);
            }
            return true;
        }
        else
        {
            CCTool.restart();            
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
        LOGI("modeCfg %s", String((CCTool.chip.modeCfg), HEX).c_str());
        LOGI("bslCfg %s", String((CCTool.chip.bslCfg), HEX).c_str());
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
    vars.zbFlashing = true;
    CCTool.nvram_reset(nvPrgs);
    logClear();
    printLogMsg("NVRAM erase finish! Restart CC2652!");
    vars.zbFlashing = false;
    vTaskDelete(NULL);
}

void flashZbUrl(String url)
{
    // zbFwCheck();
    ledControl.modeLED.mode = LED_BLINK_3Hz;
    vars.zbFlashing = true;

    // checkDNS();
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
            LOGI("%s", zbRole.c_str());

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
            //zbLedToggle();
            //delay(1000);
            //zbLedToggle();
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
    vars.zbFlashing = false;
    if (mqttCfg.enable && !vars.mqttConn)
    {
        connectToMqtt();
    }
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
    // WiFiClientSecure client;
    // client.setInsecure();
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int loadedSize = 0;
    int totalSize = 0;
    int maxRetries = 7;
    int retryCount = 0;
    int retryDelay = 500;
    bool isSuccess = false;

    while (retryCount < maxRetries && !isSuccess)
    {
        IPAddress ip;
        String host = getHostFromUrl(url);

        // DNS lookup
        if (!WiFi.hostByName(host.c_str(), ip))
        {
            printLogMsg("DNS lookup failed for host: " + host + ". Check your network connection and DNS settings.");
            retryCount = +3;
            delay(retryDelay * 3);
            continue;
        }

        if (loadedSize == 0)
        {
            CCTool.eraseFlash();
            sendEvent("ZB_FW_info", 11, String("erase"));
            printLogMsg("Erase completed!");
        }

        // http.begin(client, url);
        http.begin(url);
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
            try
            {
                if (!CCTool.beginFlash(BEGIN_ZB_ADDR, totalSize))
                {
                    http.end();
                    printLogMsg("Error initializing flash process");
                    continue;
                }
                printLogMsg("Begin flash");
            }
            catch (const std::bad_alloc &e)
            {
                char buffer[100];
                snprintf(buffer, sizeof(buffer), "Memory allocation failed: %s", e.what());
                printLogMsg(buffer);
                http.end();
                retryCount++;
                delay(retryDelay);
                continue;
            }
            catch (const std::exception &e)
            {
                char buffer[100];
                snprintf(buffer, sizeof(buffer), "Exception occurred: %s", e.what());
                printLogMsg(buffer);
                http.end();
                retryCount++;
                delay(retryDelay);
                continue;
            }
        }

        byte buffer[CCTool.TRANSFER_SIZE];
        WiFiClient *stream = http.getStreamPtr();

        while (http.connected() && loadedSize < totalSize)
        {
            try
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
            catch (const std::bad_alloc &e)
            {
                char buffer[100];
                snprintf(buffer, sizeof(buffer), "Memory allocation failed: %s", e.what());
                printLogMsg(buffer);
                http.end();
                retryCount++;
                delay(retryDelay);
                continue;
            }
            catch (const std::exception &e)
            {
                char buffer[100];
                snprintf(buffer, sizeof(buffer), "Exception occurred: %s", e.what());
                printLogMsg(buffer);
                http.end();
                retryCount++;
                delay(retryDelay);
                continue;
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

/*
#include <WiFi.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>

// Функция для извлечения хоста из URL


// Функция для получения размера файла
int getFileSize(const char* url) {

    LOGD("URL %s", url);
    const int httpsPort = 443;
    String host = getHostFromUrl(url);

    // Создание сокета
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        Serial.println("Failed to create socket");
        return -1;
    }

    // Настройка адреса сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(httpsPort);

    struct hostent* server = gethostbyname(host.c_str());
    if (server == NULL) {
        Serial.println("Failed to resolve hostname");
        close(sockfd);
        return -1;
    }

    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Установка соединения
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        Serial.println("Connection failed");
        close(sockfd);
        return -1;
    }

    // Отправка HTTP-запроса
    String request = "HEAD " + String(url) + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
    if (send(sockfd, request.c_str(), request.length(), 0) < 0) {
        Serial.println("Failed to send request");
        close(sockfd);
        return -1;
    }

    // Чтение ответа
    char buffer[1024];
    int bytes;
    int contentLength = -1;
    while ((bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = 0;
        String response(buffer);

        // Поиск заголовка Content-Length
        int index = response.indexOf("Content-Length: ");
        if (index != -1) {
            int endIndex = response.indexOf("\r\n", index);
            if (endIndex != -1) {
                String lengthStr = response.substring(index + 16, endIndex);
                contentLength = lengthStr.toInt();
                break;
            }
        }
    }

    close(sockfd);
    return contentLength;
}

// Функция для загрузки файла
bool downloadFile(const char* url, std::function<void(float)> progressShow, CCTools &CCTool, int &loadedSize, int &totalSize) {
    const int httpsPort = 443;
    String host = getHostFromUrl(url);

    // Создание сокета
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        Serial.println("Failed to create socket");
        return false;
    }

    // Настройка адреса сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(httpsPort);

    struct hostent* server = gethostbyname(host.c_str());
    if (server == NULL) {
        Serial.println("Failed to resolve hostname");
        close(sockfd);
        return false;
    }

    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    // Установка соединения
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        Serial.println("Connection failed");
        close(sockfd);
        return false;
    }

    // Отправка HTTP-запроса
    String request = "GET " + String(url) + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n";
    if (loadedSize > 0) {
        request += "Range: bytes=" + String(loadedSize) + "-\r\n";
    }
    request += "\r\n";

    if (send(sockfd, request.c_str(), request.length(), 0) < 0) {
        Serial.println("Failed to send request");
        close(sockfd);
        return false;
    }

    // Чтение ответа
    char buffer[1024];
    int bytes;
    bool headersEnded = false;
    while ((bytes = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytes] = 0;
        String response(buffer);

        if (!headersEnded) {
            int headerEnd = response.indexOf("\r\n\r\n");
            if (headerEnd != -1) {
                headersEnded = true;
                response = response.substring(headerEnd + 4);
            } else {
                continue;
            }
        }

        if (headersEnded) {
            int c = response.length();
            if (!CCTool.processFlash((byte*)response.c_str(), c)) {
                loadedSize = 0;
                close(sockfd);
                return false;
            }
            loadedSize += c;
            float percent = static_cast<float>(loadedSize) / totalSize * 100.0f;
            progressShow(percent);
        }
    }

    close(sockfd);
    return true;
}

bool eraseWriteZbUrl(const char *url, std::function<void(float)> progressShow, CCTools &CCTool)
{
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

        if (loadedSize == 0)
        {
            // Получение размера файла
            totalSize = getFileSize(url);
            if (totalSize <= 0) {
                printLogMsg("Failed to get file size");
                retryCount++;
                delay(retryDelay);
                continue;
            }

            if (!CCTool.beginFlash(BEGIN_ZB_ADDR, totalSize))
            {
                printLogMsg("Error initializing flash process");
                retryCount++;
                delay(retryDelay);
                continue;
            }
            printLogMsg("Begin flash");
        }

        if (downloadFile(url, progressShow, CCTool, loadedSize, totalSize)) {
            isSuccess = true;
        } else {
            retryCount++;
            delay(retryDelay);
        }
    }

    CCTool.restart();
    return isSuccess;
}
*/

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
