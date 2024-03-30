#include <ETH.h>

/*
BrdConfigStruct brdConfigs[] = {
    //{"BRD_NAME", addr, pwrPin, mdcPin, mdiPin, eth_phy_type, eth_clock_mode, pwrAltPin, btnPin, uartChPin, ledUsbPin, ledPwrPin, zbTxPin, zbRxPin, zbRstPin, zbBslPin},
    {"UZG-01", 0, -1, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, 5, 35, 33, 12, 14, 4, 36, 16, 32},
    {"SLZB-06", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, -1, 35, 4, 12, 14, 17, 5, 33, 32},
    {"Omilex ESP32-POE", 0, 12, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, -1, -1, -1, -1, -1, 4, 36, 16, 32},
    {"WT32-ETH01", 1, 16, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO0_IN, -1, -1, -1, -1, -1, 17, 5, 33, 32},
    {"T-Internet-POE", 0, -1, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, -1, -1, -1, -1, -1, 4, 36, 16, 32},
    {"China-GW", 0, 12, 23, 18, ETH_PHY_LAN8720, ETH_CLOCK_GPIO17_OUT, -1, -1, -1, -1, -1, 33, 32, 12, 14},
};*/


// Ethernet settings structure
struct EthConfig
{
    int addr;
    int pwrPin;
    int mdcPin;
    int mdiPin;
    eth_phy_type_t phyType;
    eth_clock_mode_t clkMode;
    int pwrAltPin;
};

// Miscellaneous settings structure
struct MistConfig {
    int btnPin;
    int btnPlr;
    int uartSelPin;
    int uartSelPlr;
    int ledUsbPin;
    int ledUsbPlr;
    int ledPwrPin;
    int ledPwrPlr;
};

// ZigBee settings structure
struct ZbConfig
{
    int txPin;
    int rxPin;
    int rstPin;
    int bslPin;
};

// Root configuration structure that includes all substructures
struct BrdConfigStruct
{
    char board[50];
    EthConfig eth;
    MistConfig mist;
    ZbConfig zb;
};

#define BOARD_CFG_CNT 6