bool zbFwCheck();
void zbHwCheck();
bool zbLedToggle();
bool zigbeeErase();
void nvPrgs(const String &inputMsg);
void zbEraseNV(void *pvParameters);
void flashZbUrl(String url);
bool eraseWriteZbUrl(const char *url, std::function<void(float)> progressShow, CCTools &CCTool);
//bool eraseWriteZbFile(const char *filePath, std::function<void(float)> progressShow, CCTools &CCTool);
