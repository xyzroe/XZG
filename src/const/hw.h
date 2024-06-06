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

// ZigBee settings structure
struct ZbConfig
{
    int txPin;
    int rxPin;
    int rstPin;
    int bslPin;
};

// Miscellaneous settings structure
struct MistConfig
{
    int btnPin;
    int btnPlr;
    int uartSelPin;
    int uartSelPlr;
    int ledModePin;
    int ledModePlr;
    int ledPwrPin;
    int ledPwrPlr;
};

// Root configuration structure that includes only configuration indices
struct BrdConfigStruct
{
    char board[50];
    int ethConfigIndex;
    int zbConfigIndex;
    int mistConfigIndex;
};

#define ETH_CFG_CNT 3
#define ZB_CFG_CNT 8
#define MIST_CFG_CNT 4
#define BOARD_CFG_CNT 14

struct ThisConfigStruct
{
    char board[50];
    EthConfig eth;
    ZbConfig zb;
    MistConfig mist;
};