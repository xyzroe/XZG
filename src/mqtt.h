
#include <AsyncMqttClient.h>

void mqttConnectSetup();
void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void executeCommand(const char *command);
void mqttPublishAvail();
void mqttPublishIo(const String &io, const String &state);
void mqttPublishState();
void mqttPublishDiscovery();