
void mqttConnectSetup();
void reconnectMqtt();
void callbackMqtt(char *topic, byte *payload, unsigned int length);
void mqttLoop();