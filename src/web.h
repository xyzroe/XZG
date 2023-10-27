#include "Arduino.h"
void initWebServer();
void webServerHandleClient();
void handleGeneral();
void handleSecurity();
void handleRoot();
void handleWifi();
void handleEther();
void handleMqtt();
void handleZigbeeBSL();
void handleZigbeeRestart();
void handleSerial();
void handleSavefile();
void handleApi();
void sendGzip(const char* contentType, const uint8_t content[], uint16_t contentLen);
void handleSysTools();
void printLogTime();
void printLogMsg(String msg);
void handleSaveParams();
bool checkAuth();
void handleZigbeeBSL();
void clearS2Buffer();
void getZbVer();
void getZbChip();

void progressFunc(unsigned int progress, unsigned int total);
void checkUpdateFirmware();
void runUpdateFirmware(uint8_t *data, size_t len);

#define UPD_FILE "https://github.com/mercenaruss/uzg-firmware/releases/latest/download/UZG-01.bin"