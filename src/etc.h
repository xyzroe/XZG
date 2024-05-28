#include <ETH.h>
#include "const/hw.h"

void getReadableTime(String &readableTime, unsigned long beginTime);

String sha1(String payloadStr);

extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

ThisConfigStruct *findBrdConfig(int searchId);

float getCPUtemp(bool clear = false);

void zigbeeRouterRejoin();
void zigbeeEnableBSL();
void zigbeeRestart();

void usbModeSet(usbMode mode);

void getDeviceID(char *arr);
void writeDefaultConfig(const char *path, DynamicJsonDocument &doc);

#define TIMEOUT_FACTORY_RESET 3

void factoryReset();

void setLedsDisable(bool all = false);
void cronTest();
void nmActivate();
void checkDNS(bool setup);
void setupCron();

void setClock(void *pvParameters);
void setTimezone(String timezone);
const char *getGmtOffsetForZone(const char *zone);
char *convertTimeToCron(const String &time);

void wgBegin();
void wgLoop();

void hnBegin();

void ledTask(void *parameter);
String getTime();

void checkEspUpdateAvail();
