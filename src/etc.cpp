#include <Arduino.h>
#include <etc.h>
#include <WiFi.h>
#include "config.h"
#include "log.h"
#include "mqtt.h"
#include "web.h"

extern struct ConfigSettingsStruct ConfigSettings;

void getReadableTime(String &readableTime, unsigned long beginTime)
{
  unsigned long currentMillis;
  unsigned long seconds;
  unsigned long minutes;
  unsigned long hours;
  unsigned long days;
  currentMillis = millis() - beginTime;
  seconds = currentMillis / 1000;
  minutes = seconds / 60;
  hours = minutes / 60;
  days = hours / 24;
  currentMillis %= 1000;
  seconds %= 60;
  minutes %= 60;
  hours %= 24;

  readableTime = String(days) + " d ";

  if (hours < 10)
  {
    readableTime += "0";
  }
  readableTime += String(hours) + ":";

  if (minutes < 10)
  {
    readableTime += "0";
  }
  readableTime += String(minutes) + ":";

  if (seconds < 10)
  {
    readableTime += "0";
  }
  readableTime += String(seconds) + "";
}

void getCPUtemp(String &CPUtemp)
{
  if (!ConfigSettings.enableWiFi && !ConfigSettings.emergencyWifi)
  {
    //DEBUG_PRINTLN(F("enable wifi to enable temp sensor "));
    WiFi.mode(WIFI_STA);
  }
  CPUtemp = (temprature_sens_read() - 32) / 1.8;
  //DEBUG_PRINT(F("CPU temp "));
  //DEBUG_PRINTLN(CPUtemp);
  if (!ConfigSettings.enableWiFi && !ConfigSettings.emergencyWifi)
  {
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    //DEBUG_PRINTLN(F("disable wifi"));
  }
}

/*
void parse_ip_address(IPAddress &ip, const char *str)
{
  IPAddress result;
  int index = 0;

  result[0] = 0;
  while (*str)
  {
    if (isdigit((unsigned char)*str))
    {
      result[index] *= 10;
      result[index] += *str - '0';
    }
    else
    {
      index++;
      if (index < 4)
      {
        result[index] = 0;
      }
    }
    str++;
  }

  ip = result;
}
*/

void zigbeeEnableBSL()
{
  printLogMsg("Zigbee BSL pin ON");
  digitalWrite(ConfigSettings.flashZigbeePin, 0);
  mqttPublishIo("enbl_bsl", "ON");
  delay(100);

  printLogMsg("Zigbee RST pin ON");
  digitalWrite(ConfigSettings.rstZigbeePin, 0);
  mqttPublishIo("rst_zig", "ON");
  delay(250);

  printLogMsg("Zigbee RST pin OFF");
  digitalWrite(ConfigSettings.rstZigbeePin, 1);
  mqttPublishIo("rst_zig", "OFF");
  delay(2000);

  printLogMsg("Zigbee BSL pin OFF");
  digitalWrite(ConfigSettings.flashZigbeePin, 1);
  mqttPublishIo("enbl_bsl", "OFF");
  printLogMsg("Now you can flash CC2652!");
}

void zigbeeReset()
{
  printLogMsg("Zigbee RST pin ON");
  digitalWrite(ConfigSettings.rstZigbeePin, 0);
  mqttPublishIo("rst_zig", "ON");
  delay(250);
  printLogMsg("Zigbee RST pin OFF");
  digitalWrite(ConfigSettings.rstZigbeePin, 1);
  mqttPublishIo("rst_zig", "OFF");
}