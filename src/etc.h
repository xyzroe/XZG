void getReadableTime(String &readableTime, unsigned long beginTime);

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();


#define STRINGIFY(s) STRINGIFY1(s) // Don’t ask why. It has to do with the inner workings of the preprocessor.
#define STRINGIFY1(s) #s // https://community.platformio.org/t/how-to-put-a-string-in-a-define-in-build-flag-into-a-libary-json-file/13480/6



float getCPUtemp(bool clear = false);

void zigbeeRouterRejoin();
void zigbeeEnableBSL();
void zigbeeRestart();

void adapterModeUSB();
void adapterModeLAN();

void ledPowerToggle();
void ledUSBToggle();

void getDeviceID(char * arr);
void writeDefultConfig(const char *path, JsonDocument& doc);

void resetSettings();

String hexToDec(String hexString);

//#define min(a, b) ((a) < (b) ? (a) : (b))