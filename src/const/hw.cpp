#include "const/hw.h"


BrdConfigStruct brdConfigs[BOARD_CFG_CNT] = {
    {"UZG-01",
     {.addr = 0, .pwrPin = -1, .mdcPin = 23, .mdiPin = 18, .phyType = ETH_PHY_LAN8720, .clkMode = ETH_CLOCK_GPIO17_OUT, .pwrAltPin = 5},
     {.btnPin = 35, .btnPlr = 1, .uartSelPin = 33, .uartSelPlr = 1, .ledUsbPin = 12, .ledUsbPlr = 1, .ledPwrPin = 14, .ledPwrPlr = 1},
     {.txPin = 4, .rxPin = 36, .rstPin = 16, .bslPin = 32}},
    {"SLZB-06",
     {.addr = 1, .pwrPin = 16, .mdcPin = 23, .mdiPin = 18, .phyType = ETH_PHY_LAN8720, .clkMode = ETH_CLOCK_GPIO0_IN, .pwrAltPin = -1},
     {.btnPin = 35, .btnPlr = 1, .uartSelPin = 4, .uartSelPlr = 1, .ledUsbPin = 12, .ledUsbPlr = 1, .ledPwrPin = 14, .ledPwrPlr = 1},
     {.txPin = 17, .rxPin = 5, .rstPin = 33, .bslPin = 32}},
    {"Omilex-ESP32-POE",
     {.addr = 0, .pwrPin = 12, .mdcPin = 23, .mdiPin = 18, .phyType = ETH_PHY_LAN8720, .clkMode = ETH_CLOCK_GPIO17_OUT, .pwrAltPin = -1},
     {.btnPin = -1, .btnPlr = 0, .uartSelPin = -1, .uartSelPlr = 0, .ledUsbPin = -1, .ledUsbPlr = 0, .ledPwrPin = -1, .ledPwrPlr = 0},
     {.txPin = 4, .rxPin = 36, .rstPin = 16, .bslPin = 32}},
    {"WT32-ETH01",
     {.addr = 1, .pwrPin = 16,  .mdcPin = 23, .mdiPin = 18, .phyType = ETH_PHY_LAN8720, .clkMode = ETH_CLOCK_GPIO0_IN, .pwrAltPin = -1},
     {.btnPin = -1, .btnPlr = 0, .uartSelPin = -1, .uartSelPlr = 0, .ledUsbPin = -1, .ledUsbPlr = 0, .ledPwrPin = -1, .ledPwrPlr = 0},
     {.txPin = 17, .rxPin = 5, .rstPin = 33, .bslPin = 32}},
    {"T-Internet-POE",
     {.addr = 0, .pwrPin = -1, .mdcPin = 23, .mdiPin = 18, .phyType = ETH_PHY_LAN8720, .clkMode = ETH_CLOCK_GPIO17_OUT, .pwrAltPin = -1},
     {.btnPin = -1, .btnPlr = 0, .uartSelPin = -1, .uartSelPlr = 0, .ledUsbPin = -1, .ledUsbPlr = 0, .ledPwrPin = -1, .ledPwrPlr = 0},
     {.txPin = 4, .rxPin = 36, .rstPin = 16, .bslPin = 32}},
    {"China-GW",
     {.addr = 0, .pwrPin = 12, .mdcPin = 23, .mdiPin = 18, .phyType = ETH_PHY_LAN8720, .clkMode = ETH_CLOCK_GPIO17_OUT, .pwrAltPin = -1},
     {.btnPin = -1, .btnPlr = 0, .uartSelPin = -1, .uartSelPlr = 0, .ledUsbPin = -1, .ledUsbPlr = 0, .ledPwrPin = -1, .ledPwrPlr = 0},
     {.txPin = 33, .rxPin = 32, .rstPin = 12, .bslPin = 14}},
};