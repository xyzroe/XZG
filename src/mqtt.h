
void mqttConnectSetup();
void mqttReconnect();
void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqttLoop();
void mqttPublishState();
void mqttOnConnect();
void mqttPublishAvty();
void mqttPublishDiscovery();
void mqttPublishMsg(String topic, String msg, bool retain);
void mqttPublishIo(String const &io, String const &state);
void mqttSubscribe(String topic);