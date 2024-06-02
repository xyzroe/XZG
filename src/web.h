#include "Arduino.h"
void initWebServer();
void webServerHandleClient();
void handleGeneral();
void handleSecurity();
void handleRoot();
void handleWifi();
void handleEther();
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

#define UPD_FILE "https://github.com/smlight-dev/SLZB-06/releases/latest/download/SLZB-06.bin"