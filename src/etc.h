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

struct BrdConfig
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
    int uartSelectPin;
    int zbTxPin;
    int zbRxPin;
    int zbRstPin;
    int zbBslPin;
};


BrdConfig *findBrdConfig();

float getCPUtemp(bool clear = false);

void zigbeeRouterRejoin();
void zigbeeEnableBSL();
void zigbeeRestart();

void adapterModeUSB();
void adapterModeLAN();

void ledPowerToggle();
void ledUSBToggle();

void getDeviceID(char *arr);
void writeDefaultConfig(const char *path, DynamicJsonDocument &doc);

void resetSettings();

String hexToDec(String hexString);

void setClock();
void setTimezone(String timezone);
const char *getGmtOffsetForZone(const char *zone);
void ledsScheduler();

// #define min(a, b) ((a) < (b) ? (a) : (b))