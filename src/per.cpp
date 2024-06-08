#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include <ETH.h>
#include <CCTools.h>
#include <esp_task_wdt.h>
#include <CronAlarms.h>
// #include <Husarnet.h> //not available now

#include "config.h"
#include "web.h"
#include "log.h"
#include "etc.h"
#include "const/zones.h"
// #include "const/hw.h"
#include "zb.h"
#include "per.h"

#include <Ticker.h>

// extern struct ConfigSettingsStruct ConfigSettings;
extern BrdConfigStruct brdConfigs[BOARD_CFG_CNT];

extern struct ThisConfigStruct hwConfig;
// extern struct CurrentModesStruct modes;

extern struct SystemConfigStruct systemCfg;
extern struct NetworkConfigStruct networkCfg;
extern struct VpnConfigStruct vpnCfg;
extern struct MqttConfigStruct mqttCfg;

extern struct SysVarsStruct vars;

extern LEDControl ledControl;

extern CCTools CCTool;

// extern int btnFlag;
int btnFlag = 0;

Ticker tmrBtnLongPress(handleLongBtn, 1000, 0, MILLIS);

void handleLongBtn()
{
    if (digitalRead(hwConfig.mist.btnPin) != hwConfig.mist.btnPlr)
    {
        LOGD("press +, %d s", btnFlag);

        btnFlag++;
        if (btnFlag >= 3)
        {
            ledControl.modeLED.mode = LED_FLASH_1Hz;
        }
    }
    else
    {
        LOGD("press -, %d s", btnFlag);

        if (btnFlag >= 3)
        {

            printLogMsg("BTN - 3sec - toggleUsbMode");
            toggleUsbMode();
        }
        else
        {
            printLogMsg("BTN - click - setLedsDisable");

            setLedsDisable(!vars.disableLeds);
            vars.disableLeds = !vars.disableLeds;
        }
        tmrBtnLongPress.stop();
        btnFlag = false;
    }
    if (btnFlag >= 5)
    {
        ledControl.modeLED.mode = LED_FLASH_3Hz;
        printLogMsg("BTN - 5sec - zigbeeEnableBSL");
        zigbeeEnableBSL();
        tmrBtnLongPress.stop();
        btnFlag = false;
    }
}

void toggleUsbMode()
{
    if (systemCfg.workMode != WORK_MODE_USB)
    {
        systemCfg.workMode = WORK_MODE_USB;
    }
    else
    {
        systemCfg.workMode = WORK_MODE_NETWORK;
    }
    saveSystemConfig(systemCfg);
    LOGD("Change mode to %s", String(systemCfg.workMode));

    if (vars.hwLedUsbIs)
    {
        ledControl.modeLED.mode = LED_ON;
    }
    ESP.restart();
}

void buttonInit()
{
    pinMode(hwConfig.mist.btnPin, INPUT);
    vars.hwBtnIs = true;
}

void buttonSetup()
{
    // hard reset BTN
    // #if BUILD_ENV_NAME != debug
    if (digitalRead(hwConfig.mist.btnPin) != hwConfig.mist.btnPlr)
    {
        LOGW("!!! Entering hard reset mode !!!");
        uint8_t counter = 0;
        while (digitalRead(hwConfig.mist.btnPin) != hwConfig.mist.btnPlr)
        {
            if (counter >= 10)
            {
                factoryReset();
            }
            else
            {
                counter++;
                LOGW("%d", counter);
                delay(200);
            }
        }
        LOGI("Btn released, so exit");
    }
    // #endif
    if (hwConfig.mist.btnPlr)
    {
        attachInterrupt(digitalPinToInterrupt(hwConfig.mist.btnPin), btnInterrupt, FALLING);
    }
    else
    {
        attachInterrupt(digitalPinToInterrupt(hwConfig.mist.btnPin), btnInterrupt, RISING);
    }
}

void buttonLoop()
{
    if (digitalRead(hwConfig.mist.btnPin) != hwConfig.mist.btnPlr) // pressed
    {
        if (tmrBtnLongPress.state() == STOPPED)
        {
            tmrBtnLongPress.start();
        }
    }
    tmrBtnLongPress.update();
}

IRAM_ATTR bool debounce()
{
    volatile static unsigned long lastFire = 0;
    if (millis() - lastFire < DEBOUNCE_TIME)
    { // Debounce
        return 0;
    }
    lastFire = millis();
    return 1;
}

IRAM_ATTR void btnInterrupt()
{
    if (debounce())
    {
        if (!btnFlag)
        {
            btnFlag = true;
        }
    }
}

void ledModeSetup()
{
    pinMode(hwConfig.mist.ledModePin, OUTPUT);
    vars.hwLedUsbIs = true;

    ledControl.modeLED.name = "Mode";
    ledControl.modeLED.pin = hwConfig.mist.ledModePin;
    ledControl.modeLED.active = true;
    ledControl.modeLED.mode = LED_OFF;

    LOGD("%d", ledControl.modeLED.mode);

    xTaskCreate(ledTask, "MODE LED Task", 2048, &ledControl.modeLED, 7, NULL);
}

void ledPwrSetup()
{
    pinMode(hwConfig.mist.ledPwrPin, OUTPUT);
    vars.hwLedPwrIs = true;

    ledControl.powerLED.name = "Power";
    ledControl.powerLED.pin = hwConfig.mist.ledPwrPin;
    ledControl.powerLED.active = true;
    ledControl.powerLED.mode = LED_OFF;

    LOGD("%d", ledControl.powerLED.mode);

    xTaskCreate(ledTask, "PWR LED Task", 2048, &ledControl.powerLED, 6, NULL);
}