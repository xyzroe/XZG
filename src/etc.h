#include <ETH.h>
#include "hw.h"

void getReadableTime(String &readableTime, unsigned long beginTime);

String sha1(String payloadStr);

extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

BrdConfigStruct *findBrdConfig(int searchId);

float getCPUtemp(bool clear = false);

void zigbeeRouterRejoin();
void zigbeeEnableBSL();
void zigbeeRestart();

void adapterModeUSB();
void adapterModeLAN();

void getDeviceID(char *arr);
void writeDefaultConfig(const char *path, DynamicJsonDocument &doc);

void factoryReset();

//String hexToDec(String hexString);

void setLedsDisable(bool all = false);
void cronTest();
void nmActivate();
void setupCron();

void setClock(void *pvParameters);
void setTimezone(String timezone);
const char *getGmtOffsetForZone(const char *zone);
char* convertTimeToCron(const String& time);

void ledsScheduler();

void wgBegin();
void wgLoop();

void hnBegin();

void ledTask(void *parameter);
String getTime();

