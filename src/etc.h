#include <ETH.h>
#include "hw.h"

void getReadableTime(String &readableTime, unsigned long beginTime);

#ifdef __cplusplus
extern "C"
{
#endif
    uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

#define STRINGIFY(s) STRINGIFY1(s) // Don’t ask why. It has to do with the inner workings of the preprocessor.
#define STRINGIFY1(s) #s           // https://community.platformio.org/t/how-to-put-a-string-in-a-define-in-build-flag-into-a-libary-json-file/13480/6

#define MAX_DEV_ID_LONG 50
/*
struct CurrentModesStruct
{
    bool btnIs = false;
    bool ledUsbIs = false;
    bool ledPwrIs = false;
    bool uartChIs = false;
    bool zigbeeIs = false;
};*/

/*
struct BrdConfigStruct
{
    char board[50];
    int addr;
    int pwrPin;
    int mdcPin;
    int mdiPin;
    eth_phy_type_t phyType;
    eth_clock_mode_t clkMode;
    int pwrAltPin;
    int btnPin;
    int uartChPin;
    int ledUsbPin;
    int ledPwrPin;
    int zbTxPin;
    int zbRxPin;
    int zbRstPin;
    int zbBslPin;
};*/

extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

BrdConfigStruct *findBrdConfig(int searchId);

float getCPUtemp(bool clear = false);

void zigbeeRouterRejoin();
void zigbeeEnableBSL();
void zigbeeRestart();

void adapterModeUSB();
void adapterModeLAN();

void ledPwrToggle();
void ledUsbToggle();

void getDeviceID(char *arr);
void writeDefaultConfig(const char *path, DynamicJsonDocument &doc);

void resetSettings();

//String hexToDec(String hexString);

void setClock();
void setTimezone(String timezone);
const char *getGmtOffsetForZone(const char *zone);
void ledsScheduler();

void wgBegin();
void wgLoop();

// #define min(a, b) ((a) < (b) ? (a) : (b))