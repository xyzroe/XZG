void getReadableTime(String &readableTime, unsigned long beginTime);

#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

void getCPUtemp(String &CPUtemp);
void getBlankCPUtemp(String &CPUtemp);
//void parse_ip_address(IPAddress &ip, const char *str)
void zigbeeEnableBSL();
void zigbeeRestart();

void getDeviceID(String &devID);
void writeDefultConfig(const char *path, String StringConfig);
String hexToDec(String hexString);

#define min(a, b) ((a) < (b) ? (a) : (b))