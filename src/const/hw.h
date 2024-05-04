#include <ETH.h>

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
    int ledModePin;
    int ledModePlr;
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