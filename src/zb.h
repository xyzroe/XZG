void zbFwCheck();
void zbHwCheck();
void zbLedToggle();
bool zigbeeErase();
void flashZbUrl(String url);
bool eraseWriteZbUrl(const char *url, std::function<void(float)> progressShow, CCTools &CCTool);
bool eraseWriteZbFile(const char *filePath, std::function<void(float)> progressShow, CCTools &CCTool);
