#include <Arduino.h>
void handleEvents();
void initWebServer();
void webServerHandleClient();
void handleGeneral();
void handleSecurity();
void handleRoot();
void handleWifi();
void handleEther();
void handleMqtt();
void handleWg();
void handleZigbeeBSL();
void handleZigbeeRestart();
void handleSerial();
void handleSavefile();
void handleApi();
void handleStatus();
void sendGzip(const char* contentType, const uint8_t content[], uint16_t contentLen);
void handleSysTools();
void printLogTime();
void printLogMsg(String msg);
void handleSaveParams();
bool checkAuth();

void sendEvent(const char *event, const uint8_t evsz, const String data);
void progressFunc(unsigned int progress, unsigned int total);

void getEspUpdate(String esp_fw_url);
void runEspUpdateFirmware(uint8_t *data, size_t len);

#define UPD_FILE "https://github.com/mercenaruss/uzg-firmware/releases/latest/download/UZG-01.bin"