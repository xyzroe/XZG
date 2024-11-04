
#include <AsyncMqttClient.h>

void mqttConnectSetup();
void mqttDisconnectCleanup();
void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
void executeCommand(const char *command);
void mqttPublishAvail();
void mqttPublishIo(const String &io, bool st);
void mqttPublishUpdate(const String &chip, bool state = false);
void mqttPublishState();
void mqttPublishDiscovery();

typedef String (*SensorValueFunction)();

struct mqttTopicsConfig {
    String name;
    String sensorType;
    String sensorId;
    String stateTopic;
    String commandTopic;
    String icon;
    String payloadPress;
    String valueTemplate;
    String deviceClass;
    String unitOfMeasurement;
    String entityCategory;
    String entityPicture;
    String payloadInstall;
    String releaseUrl;
    String jsonAttrTemplate;
    String jsonAttrTopic;
    SensorValueFunction getSensorValue;
};