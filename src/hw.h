#include <ETH.h>

BrdConfigStruct brdConfigs[] = {
    //{"BRD_NAME", addr, pwrPin, mdcPin, mdiPin, eth_phy_type, eth_clock_mode, pwrAltPin, btnPin, uartChPin, ledUsbPin, ledPwrPin, zbTxPin, zbRxPin, zbRstPin, zbBslPin},
    {"UZG-01", 0, -1, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, 5, 35, 33, 12, 14, 4, 36, 16, 32},
    {"SLZB-06", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, -1, 35, 4, 12, 14, 17, 5, 33, 32},
    {"Omilex ESP32-POE", 0, 12, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, -1, -1, -1, -1, -1, 4, 36, 16, 32},
    {"WT32-ETH01", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, -1, -1, -1, -1, -1, 17, 5, 33, 32},
    {"T-Internet-POE", 0, -1, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, -1, -1, -1, -1, -1, 4, 36, 16, 32},
    {"China-GW", 0, 12, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, -1, -1, -1, -1, -1, 33, 32, 12, 14},
};