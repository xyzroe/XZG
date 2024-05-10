#ifndef CCTools_DETECT_H
#define CCTools_DETECT_H

#include <Arduino.h>

#ifndef CHIP_IDs_H
#define CHIP_IDs_H
enum class ChipId
{
    CC2652P2,
    CC2652P7,
    Unknown
};
#endif

enum OsalNvIds
{
    // Introduced by zigbeer/zigbee-shepherd and now used by Zigbee2MQTT,
    HAS_CONFIGURED_ZSTACK1 = 0x0F00,
    HAS_CONFIGURED_ZSTACK3 = 0x0060,

    // Although the docs say "IDs reserved for applications range from 0x0401 to 0x0FFF",,
    // no OSAL NVID beyond 0x03FF is writable with the MT interface when using Z-Stack 3.,
    ZIGPY_ZNP_MIGRATION_ID = 0x005F,

    // OSAL NV item IDs,
    EXTADDR = 0x0001,
    BOOTCOUNTER = 0x0002,
    STARTUP_OPTION = 0x0003,
    START_DELAY = 0x0004,

    // NWK Layer NV item IDs,
    NIB = 0x0021,
    DEVICE_LIST = 0x0022,
    ADDRMGR = 0x0023,
    POLL_RATE_OLD16 = 0x0024, // Deprecated when poll rate changed from 16 to 32 bits,
    QUEUED_POLL_RATE = 0x0025,
    RESPONSE_POLL_RATE = 0x0026,
    REJOIN_POLL_RATE = 0x0027,
    DATA_RETRIES = 0x0028,
    POLL_FAILURE_RETRIES = 0x0029,
    STACK_PROFILE = 0x002A,
    INDIRECT_MSG_TIMEOUT = 0x002B,
    ROUTE_EXPIRY_TIME = 0x002C,
    EXTENDED_PAN_ID = 0x002D,
    BCAST_RETRIES = 0x002E,
    PASSIVE_ACK_TIMEOUT = 0x002F,
    BCAST_DELIVERY_TIME = 0x0030,
    NWK_MODE = 0x0031, // Deprecated, as this will always be Mesh,
    CONCENTRATOR_ENABLE = 0x0032,
    CONCENTRATOR_DISCOVERY = 0x0033,
    CONCENTRATOR_RADIUS = 0x0034,
    POLL_RATE = 0x0035,
    CONCENTRATOR_RC = 0x0036,
    NWK_MGR_MODE = 0x0037,
    SRC_RTG_EXPIRY_TIME = 0x0038,
    ROUTE_DISCOVERY_TIME = 0x0039,
    NWK_ACTIVE_KEY_INFO = 0x003A,
    NWK_ALTERN_KEY_INFO = 0x003B,
    ROUTER_OFF_ASSOC_CLEANUP = 0x003C,
    NWK_LEAVE_REQ_ALLOWED = 0x003D,
    NWK_CHILD_AGE_ENABLE = 0x003E,
    DEVICE_LIST_KA_TIMEOUT = 0x003F,

    // APS Layer NV item IDs,
    BINDING_TABLE = 0x0041,
    GROUP_TABLE = 0x0042,
    APS_FRAME_RETRIES = 0x0043,
    APS_ACK_WAIT_DURATION = 0x0044,
    APS_ACK_WAIT_MULTIPLIER = 0x0045,
    BINDING_TIME = 0x0046,
    APS_USE_EXT_PANID = 0x0047,
    APS_USE_INSECURE_JOIN = 0x0048,
    COMMISSIONED_NWK_ADDR = 0x0049,

    APS_NONMEMBER_RADIUS = 0x004B, // Multicast non_member radius,
    APS_LINK_KEY_TABLE = 0x004C,
    APS_DUPREJ_TIMEOUT_INC = 0x004D,
    APS_DUPREJ_TIMEOUT_COUNT = 0x004E,
    APS_DUPREJ_TABLE_SIZE = 0x004F,

    // System statistics and metrics NV ID,
    DIAGNOSTIC_STATS = 0x0050,

    // Additional NWK Layer NV item IDs,
    NWK_PARENT_INFO = 0x0051,
    NWK_ENDDEV_TIMEOUT_DEF = 0x0052,
    END_DEV_TIMEOUT_VALUE = 0x0053,
    END_DEV_CONFIGURATION = 0x0054,

    BDBNODEISONANETWORK = 0x0055, // bdbNodeIsOnANetwork attribute,
    BDBREPORTINGCONFIG = 0x0056,

    // Security NV Item IDs,
    SECURITY_LEVEL = 0x0061,
    PRECFGKEY = 0x0062,
    PRECFGKEYS_ENABLE = 0x0063,
    // Deprecated Item as there is only one security mode supported now Z3.0,
    SECURITY_MODE = 0x0064,
    SECURE_PERMIT_JOIN = 0x0065,
    APS_LINK_KEY_TYPE = 0x0066,
    APS_ALLOW_R19_SECURITY = 0x0067,
    DISTRIBUTED_KEY = 0x0068, // Default distributed nwk key Id. Nv ID not in use,

    IMPLICIT_CERTIFICATE = 0x0069,
    DEVICE_PRIVATE_KEY = 0x006A,
    CA_PUBLIC_KEY = 0x006B,
    KE_MAX_DEVICES = 0x006C,

    USE_DEFAULT_TCLK = 0x006D,
    // deprecated: TRUSTCENTER_ADDR (16-bit)   0x006E,
    RNG_COUNTER = 0x006F,
    RANDOM_SEED = 0x0070,
    TRUSTCENTER_ADDR = 0x0071,

    CERT_283 = 0x0072,
    PRIVATE_KEY_283 = 0x0073,
    PUBLIC_KEY_283 = 0x0074,

    LEGACY_NWK_SEC_MATERIAL_TABLE_START = 0x0075,
    LEGACY_NWK_SEC_MATERIAL_TABLE_END = 0x0080,

    // ZDO NV Item IDs,
    USERDESC = 0x0081,
    NWKKEY = 0x0082,
    PANID = 0x0083,
    CHANLIST = 0x0084,
    LEAVE_CTRL = 0x0085,
    SCAN_DURATION = 0x0086,
    LOGICAL_TYPE = 0x0087,
    NWKMGR_MIN_TX = 0x0088,
    NWKMGR_ADDR = 0x0089,

    ZDO_DIRECT_CB = 0x008F,

    // ZCL NV item IDs,
    SCENE_TABLE = 0x0091,
    MIN_FREE_NWK_ADDR = 0x0092,
    MAX_FREE_NWK_ADDR = 0x0093,
    MIN_FREE_GRP_ID = 0x0094,
    MAX_FREE_GRP_ID = 0x0095,
    MIN_GRP_IDS = 0x0096,
    MAX_GRP_IDS = 0x0097,
    OTA_BLOCK_REQ_DELAY = 0x0098,

    // Non-standard NV item IDs,
    SAPI_ENDPOINT = 0x00A1,

    // NV Items Reserved for Commissioning Cluster Startup Attribute Set (SAS):,
    // 0x00B1 - 0x00BF: Parameters related to APS and NWK layers,
    SAS_SHORT_ADDR = 0x00B1,
    SAS_EXT_PANID = 0x00B2,
    SAS_PANID = 0x00B3,
    SAS_CHANNEL_MASK = 0x00B4,
    SAS_PROTOCOL_VER = 0x00B5,
    SAS_STACK_PROFILE = 0x00B6,
    SAS_STARTUP_CTRL = 0x00B7,

    // 0x00C1 - 0x00CF: Parameters related to Security,
    SAS_TC_ADDR = 0x00C1,
    SAS_TC_MASTER_KEY = 0x00C2,
    SAS_NWK_KEY = 0x00C3,
    SAS_USE_INSEC_JOIN = 0x00C4,
    SAS_PRECFG_LINK_KEY = 0x00C5,
    SAS_NWK_KEY_SEQ_NUM = 0x00C6,
    SAS_NWK_KEY_TYPE = 0x00C7,
    SAS_NWK_MGR_ADDR = 0x00C8,

    // 0x00D1 - 0x00DF: Current key parameters,
    SAS_CURR_TC_MASTER_KEY = 0x00D1,
    SAS_CURR_NWK_KEY = 0x00D2,
    SAS_CURR_PRECFG_LINK_KEY = 0x00D3,

    USE_NVOCMP = 0x00FF,

    // NV Items Reserved for Trust Center Link Key Table entries,
    // 0x0101 - 0x01FF,
    TCLK_SEED = 0x0101,     // Seed,
    TCLK_JOIN_DEV = 0x0102, // Nv Id where Joining device store their APS key. Key is in plain text.,

    TCLK_DEFAULT = 0x0103, // Not accually a Nv Item but Id used by SecMgr,

    LEGACY_TCLK_IC_TABLE_START = 0x0104, // Deprecated. Refer to EX_TCLK_IC_TABLE,
    LEGACY_TCLK_IC_TABLE_END = 0x0110,   // IC keys, referred with shift byte,

    LEGACY_TCLK_TABLE_START = 0x0111, // Deprecated. Refer to EX_TCLK_TABLE,
    LEGACY_TCLK_TABLE_END = 0x01FF,

    // NV Items Reserved for APS Link Key Table entries,
    // 0x0201 - 0x02FF,
    LEGACY_APS_LINK_KEY_DATA_START = 0x0201, // Deprecated. Refer to EX_APS_KEY_TABLE,
    LEGACY_APS_LINK_KEY_DATA_END = 0x02FF,

    // NV items used to duplicate system elements,
    DUPLICATE_BINDING_TABLE = 0x0300,
    DUPLICATE_DEVICE_LIST = 0x0301,
    DUPLICATE_DEVICE_LIST_KA_TIMEOUT = 0x0302,

    // NV Items Reserved for Proxy Table entries,
    // 0x0310 - 0x031F,
    LEGACY_PROXY_TABLE_START = 0x0310, // Deprecated. Refer to EX_GP_PROXY_TABLE,
    LEGACY_PROXY_TABLE_END = 0x031F,

    // NV Items Reserved for Sink Table entries,
    // 0x0320 - 0x032F,
    LEGACY_SINK_TABLE_START = 0x0320, // Deprecated. Refer to EX_GP_SINK_TABLE,
    LEGACY_SINK_TABLE_END = 0x032F,

    APP_ITEM_1 = 0x0F01,
    APP_ITEM_2 = 0x0F02,
    APP_ITEM_3 = 0x0F03,
    APP_ITEM_4 = 0x0F04,
    APP_ITEM_5 = 0x0F05,
    APP_ITEM_6 = 0x0F06,

    RF_TEST_PARMS = 0x0F07,

    UNKNOWN = 0x0F08,

    INVALID_INDEX = 0xFFFF
};

const OsalNvIds keys[] =
    {
        OsalNvIds::HAS_CONFIGURED_ZSTACK1,
        OsalNvIds::HAS_CONFIGURED_ZSTACK3,
        OsalNvIds::ZIGPY_ZNP_MIGRATION_ID,
        OsalNvIds::EXTADDR,
        OsalNvIds::BOOTCOUNTER,
        OsalNvIds::STARTUP_OPTION,
        OsalNvIds::START_DELAY,
        OsalNvIds::NIB,
        OsalNvIds::DEVICE_LIST,
        OsalNvIds::ADDRMGR,
        OsalNvIds::POLL_RATE_OLD16,
        OsalNvIds::QUEUED_POLL_RATE,
        OsalNvIds::RESPONSE_POLL_RATE,
        OsalNvIds::REJOIN_POLL_RATE,
        OsalNvIds::DATA_RETRIES,
        OsalNvIds::POLL_FAILURE_RETRIES,
        OsalNvIds::STACK_PROFILE,
        OsalNvIds::INDIRECT_MSG_TIMEOUT,
        OsalNvIds::ROUTE_EXPIRY_TIME,
        OsalNvIds::EXTENDED_PAN_ID,
        OsalNvIds::BCAST_RETRIES,
        OsalNvIds::PASSIVE_ACK_TIMEOUT,
        OsalNvIds::BCAST_DELIVERY_TIME,
        OsalNvIds::NWK_MODE,
        OsalNvIds::CONCENTRATOR_ENABLE,
        OsalNvIds::CONCENTRATOR_DISCOVERY,
        OsalNvIds::CONCENTRATOR_RADIUS,
        OsalNvIds::POLL_RATE,
        OsalNvIds::CONCENTRATOR_RC,
        OsalNvIds::NWK_MGR_MODE,
        OsalNvIds::SRC_RTG_EXPIRY_TIME,
        OsalNvIds::ROUTE_DISCOVERY_TIME,
        OsalNvIds::NWK_ACTIVE_KEY_INFO,
        OsalNvIds::NWK_ALTERN_KEY_INFO,
        OsalNvIds::ROUTER_OFF_ASSOC_CLEANUP,
        OsalNvIds::NWK_LEAVE_REQ_ALLOWED,
        OsalNvIds::NWK_CHILD_AGE_ENABLE,
        OsalNvIds::DEVICE_LIST_KA_TIMEOUT,
        OsalNvIds::BINDING_TABLE,
        OsalNvIds::GROUP_TABLE,
        OsalNvIds::APS_FRAME_RETRIES,
        OsalNvIds::APS_ACK_WAIT_DURATION,
        OsalNvIds::APS_ACK_WAIT_MULTIPLIER,
        OsalNvIds::BINDING_TIME,
        OsalNvIds::APS_USE_EXT_PANID,
        OsalNvIds::APS_USE_INSECURE_JOIN,
        OsalNvIds::COMMISSIONED_NWK_ADDR,
        OsalNvIds::APS_NONMEMBER_RADIUS,
        OsalNvIds::APS_LINK_KEY_TABLE,
        OsalNvIds::APS_DUPREJ_TIMEOUT_INC,
        OsalNvIds::APS_DUPREJ_TIMEOUT_COUNT,
        OsalNvIds::APS_DUPREJ_TABLE_SIZE,
        OsalNvIds::DIAGNOSTIC_STATS,
        OsalNvIds::NWK_PARENT_INFO,
        OsalNvIds::NWK_ENDDEV_TIMEOUT_DEF,
        OsalNvIds::END_DEV_TIMEOUT_VALUE,
        OsalNvIds::END_DEV_CONFIGURATION,
        OsalNvIds::BDBNODEISONANETWORK,
        OsalNvIds::BDBREPORTINGCONFIG,
        OsalNvIds::SECURITY_LEVEL,
        OsalNvIds::PRECFGKEY,
        OsalNvIds::PRECFGKEYS_ENABLE,
        OsalNvIds::SECURITY_MODE,
        OsalNvIds::SECURE_PERMIT_JOIN,
        OsalNvIds::APS_LINK_KEY_TYPE,
        OsalNvIds::APS_ALLOW_R19_SECURITY,
        OsalNvIds::DISTRIBUTED_KEY,
        OsalNvIds::IMPLICIT_CERTIFICATE,
        OsalNvIds::DEVICE_PRIVATE_KEY,
        OsalNvIds::CA_PUBLIC_KEY,
        OsalNvIds::KE_MAX_DEVICES,
        OsalNvIds::USE_DEFAULT_TCLK,
        OsalNvIds::RNG_COUNTER,
        OsalNvIds::RANDOM_SEED,
        OsalNvIds::TRUSTCENTER_ADDR,
        OsalNvIds::CERT_283,
        OsalNvIds::PRIVATE_KEY_283,
        OsalNvIds::PUBLIC_KEY_283,
        OsalNvIds::LEGACY_NWK_SEC_MATERIAL_TABLE_START,
        OsalNvIds::LEGACY_NWK_SEC_MATERIAL_TABLE_END,
        OsalNvIds::USERDESC,
        OsalNvIds::NWKKEY,
        OsalNvIds::PANID,
        OsalNvIds::CHANLIST,
        OsalNvIds::LEAVE_CTRL,
        OsalNvIds::SCAN_DURATION,
        OsalNvIds::LOGICAL_TYPE,
        OsalNvIds::NWKMGR_MIN_TX,
        OsalNvIds::NWKMGR_ADDR,
        OsalNvIds::ZDO_DIRECT_CB,
        OsalNvIds::SCENE_TABLE,
        OsalNvIds::MIN_FREE_NWK_ADDR,
        OsalNvIds::MAX_FREE_NWK_ADDR,
        OsalNvIds::MIN_FREE_GRP_ID,
        OsalNvIds::MAX_FREE_GRP_ID,
        OsalNvIds::MIN_GRP_IDS,
        OsalNvIds::MAX_GRP_IDS,
        OsalNvIds::OTA_BLOCK_REQ_DELAY,
        OsalNvIds::SAPI_ENDPOINT,
        OsalNvIds::SAS_SHORT_ADDR,
        OsalNvIds::SAS_EXT_PANID,
        OsalNvIds::SAS_PANID,
        OsalNvIds::SAS_CHANNEL_MASK,
        OsalNvIds::SAS_PROTOCOL_VER,
        OsalNvIds::SAS_STACK_PROFILE,
        OsalNvIds::SAS_STARTUP_CTRL,
        OsalNvIds::SAS_TC_ADDR,
        OsalNvIds::SAS_TC_MASTER_KEY,
        OsalNvIds::SAS_NWK_KEY,
        OsalNvIds::SAS_USE_INSEC_JOIN,
        OsalNvIds::SAS_PRECFG_LINK_KEY,
        OsalNvIds::SAS_NWK_KEY_SEQ_NUM,
        OsalNvIds::SAS_NWK_KEY_TYPE,
        OsalNvIds::SAS_NWK_MGR_ADDR,
        OsalNvIds::SAS_CURR_TC_MASTER_KEY,
        OsalNvIds::SAS_CURR_NWK_KEY,
        OsalNvIds::SAS_CURR_PRECFG_LINK_KEY,
        OsalNvIds::USE_NVOCMP,
        OsalNvIds::TCLK_SEED,
        OsalNvIds::TCLK_JOIN_DEV,
        OsalNvIds::TCLK_DEFAULT,
        OsalNvIds::LEGACY_TCLK_IC_TABLE_START,
        OsalNvIds::LEGACY_TCLK_IC_TABLE_END,
        OsalNvIds::LEGACY_TCLK_TABLE_START,
        OsalNvIds::LEGACY_TCLK_TABLE_END,
        OsalNvIds::LEGACY_APS_LINK_KEY_DATA_START,
        OsalNvIds::LEGACY_APS_LINK_KEY_DATA_END,
        OsalNvIds::DUPLICATE_BINDING_TABLE,
        OsalNvIds::DUPLICATE_DEVICE_LIST,
        OsalNvIds::DUPLICATE_DEVICE_LIST_KA_TIMEOUT,
        OsalNvIds::LEGACY_PROXY_TABLE_START,
        OsalNvIds::LEGACY_PROXY_TABLE_END,
        OsalNvIds::LEGACY_SINK_TABLE_START,
        OsalNvIds::LEGACY_SINK_TABLE_END,
        OsalNvIds::APP_ITEM_1,
        OsalNvIds::APP_ITEM_2,
        OsalNvIds::APP_ITEM_3,
        OsalNvIds::APP_ITEM_4,
        OsalNvIds::APP_ITEM_5,
        OsalNvIds::APP_ITEM_6,
        OsalNvIds::RF_TEST_PARMS,
        OsalNvIds::UNKNOWN,
        OsalNvIds::INVALID_INDEX};

enum ExNvIds
{
    LEGACY = 0x0000,
    ADDRMGR_EX = 0x0001,
    BINDING_TABLE_EX = 0x0002,
    DEVICE_LIST_EX = 0x0003,
    TCLK_TABLE = 0x0004,
    TCLK_IC_TABLE = 0x0005,
    APS_KEY_DATA_TABLE = 0x0006,
    NWK_SEC_MATERIAL_TABLE = 0x0007
};

const ExNvIds exIds[] =
    {
        ExNvIds::LEGACY,
        ExNvIds::ADDRMGR_EX,
        ExNvIds::BINDING_TABLE_EX,
        ExNvIds::DEVICE_LIST_EX,
        ExNvIds::TCLK_TABLE,
        ExNvIds::TCLK_IC_TABLE,
        ExNvIds::APS_KEY_DATA_TABLE,
        ExNvIds::NWK_SEC_MATERIAL_TABLE};

const std::pair<uint16_t, uint16_t> NWK_NVID_TABLES[] = {
    {static_cast<uint16_t>(OsalNvIds::LEGACY_NWK_SEC_MATERIAL_TABLE_START), static_cast<uint16_t>(OsalNvIds::LEGACY_NWK_SEC_MATERIAL_TABLE_END)},
    {static_cast<uint16_t>(OsalNvIds::LEGACY_TCLK_IC_TABLE_START), static_cast<uint16_t>(OsalNvIds::LEGACY_TCLK_IC_TABLE_END)},
    {static_cast<uint16_t>(OsalNvIds::LEGACY_TCLK_TABLE_START), static_cast<uint16_t>(OsalNvIds::LEGACY_TCLK_TABLE_END)},
    {static_cast<uint16_t>(OsalNvIds::LEGACY_APS_LINK_KEY_DATA_START), static_cast<uint16_t>(OsalNvIds::LEGACY_APS_LINK_KEY_DATA_END)},
    {static_cast<uint16_t>(OsalNvIds::LEGACY_PROXY_TABLE_START), static_cast<uint16_t>(OsalNvIds::LEGACY_PROXY_TABLE_END)},
    {static_cast<uint16_t>(OsalNvIds::LEGACY_SINK_TABLE_START), static_cast<uint16_t>(OsalNvIds::LEGACY_SINK_TABLE_END)}};

#define BEGIN_ZB_ADDR 0x00000000

class CommandInterface
{
protected:
    Stream &_stream;

    struct zbInfoStruct
    {
        String hwRev;
        uint32_t fwRev;
        String ieee;
        uint32_t flashSize;
        uint32_t ramSize;
        uint8_t maintrel;
        uint8_t minorrel;
        uint8_t majorrel;
        uint8_t product;
        uint8_t transportrev;
    };

    static const uint8_t ACK_BYTE = 0xCC;
    static const uint8_t NACK_BYTE = 0x33;
    static const uint8_t COMMAND_RET_SUCCESS = 0x40;

    static const uint32_t ICEPICK_DEVICE_ID = 0x50001318;
    static const uint32_t FCFG_USER_ID = 0x50001294;
    static const uint32_t PRCM_RAMHWOPT = 0x40082250;
    static const uint32_t FLASH_SIZE = 0x4003002C;
    // static const uint32_t PROTO_MASK_BLE = 0x01;
    static const uint32_t PROTO_MASK_IEEE = 0x04;
    // static const uint32_t PROTO_MASK_BOTH = 0x05;

    // static const uint32_t flash_start_addr = 0x00000000;

    static const uint32_t addr_ieee_address_primary = 0x500012F0;
    // static const uint32_t ccfg_len = 88;
    static const uint32_t ieee_address_secondary_offset = 0x20;
    // static const uint32_t bootloader_dis_offset = 0x30;

    const char *_getStatusString(uint8_t statusCode)
    {
        switch (statusCode)
        {
        case 0x40:
            return "Success";
        case 0x41:
            return "Unknown command";
        case 0x42:
            return "Invalid command";
        case 0x43:
            return "Invalid address";
        case 0x44:
            return "Flash fail";
        default:
            return "Unknown";
        }
    }

    const char *_getChipDescription(uint32_t chip_id, uint32_t wafer_id, uint8_t pg_rev, uint8_t mode_cfg)
    {

        if (chip_id == 0xb964 || chip_id == 0xb965)
        {
            return "CC2538";
        }
        /*
        else if (chip_id == 0xf000)
        {
            return "CC2652";
        }
        else if (wafer_id == 0xB99A) {
            return "cc26xx";
        }
        else if (wafer_id == 0xB9BE) {
            return "cc13xx";
        }
        else if (wafer_id == 0xBB7A) {
            return "cc13xx 8192";
        }
        */
        else if (chip_id == 0x1202 && wafer_id == 0xBB77 && pg_rev == 0x1)
        {
            return "CC2652P7";
        }
        else if (chip_id == 0x3202 && wafer_id == 0xBB41 && pg_rev == 0x3 && mode_cfg == 0xC1)
        {
            return "CC2652P2_launchpad";
        }
        else if (chip_id == 0x3202 && wafer_id == 0xBB41 && pg_rev == 0x3 && mode_cfg == 0xFA)
        {
            return "CC2652P2_other";
        }
        else if (chip_id == 0x3202 && wafer_id == 0xBB41 && pg_rev == 0x3)
        {
            return "CC2652P2_";
        }
        else if (chip_id == 0x3102 && wafer_id == 0xBB41 && pg_rev == 0x3)
        {
            return "CC2652RB";
        }
        else
        {
            static char unknownDescription[50];
            sprintf(unknownDescription, "Unknown (C: %X, W: %X, P: %X, M: %X)",
                    chip_id, wafer_id, pg_rev, mode_cfg);
            return unknownDescription;
        }
    }

    const char *_getPackageString(byte user_id)
    {
        byte packageCode = user_id & 0x03; // Получаем младшие 2 бита user_id

        switch (packageCode)
        {
        case 0x00:
            return "4x4mm";
        case 0x01:
            return "5x5mm";
        case 0x02:
            return "7x7mm";
        case 0x03:
            return "Wafer";
        case 0x04:
            return "2.7x2.7";
        case 0x05:
            return "7x7mm Q1";
        default:
            return "Unknown";
        }
    }

    const byte cmdFrameStart = 0xFE;
    const byte zero = 0x00;
    const byte one = 0x01;

    const byte cmdVer1 = 0x21;
    const byte cmdVer2 = 0x02;
    const byte cmdSysVersion[5] = {cmdFrameStart, zero, cmdVer1, cmdVer2, 0x23};

    const byte cmdLed0 = 0x27;
    const byte cmdLed1 = 0x0A;
    const byte cmdLedIndex = 0x01; // for led 1
    const byte cmdLedLen = 0x02;

    const byte zigLed1Off[7] = {cmdFrameStart, cmdLedLen, cmdLed0, cmdLed1, cmdLedIndex, zero, 0x2E}; // resp FE 01 67 0A 00 6C
    const byte zigLed1On[7] = {cmdFrameStart, cmdLedLen, cmdLed0, cmdLed1, cmdLedIndex, one, 0x2F};
    const byte cmdLedResp[6] = {cmdFrameStart, 0x01, 0x67, 0x0A, zero, 0x6C};

    void _cleanBuffer();

    CommandInterface(Stream &serial);

    bool _sendSynch();
    bool _wait_for_ack(unsigned long timeout);
    byte *_receive_SRSP(unsigned long timeout);
    uint32_t _cmdGetChipId();
    byte *_cmdGetStatus();
    bool _checkLastCmd();
    void _sendAck();
    void _sendNAck();
    bool _eraseFlash();
    bool _cmdDownload(uint32_t address, unsigned long size);
    bool _cmdSendData(byte *data, unsigned int dataSize);
    bool _ping();
    byte *_receivePacket();
    void _encodeAddr(unsigned long addr, byte encodedAddr[4]);
    unsigned long _decodeAddr(byte byte0, byte byte1, byte byte2, byte byte3);
    byte _calcChecks(byte cmd, unsigned long addr, unsigned long size);
    byte *_cmdMemRead(uint32_t address);
    bool _ledToggle(bool ledState);
    zbInfoStruct _checkFwVer();
    bool _nvram_osal_delete(uint16_t nvid);
    bool _nvram_ex_delete(uint16_t nvid, uint16_t subID);
};

class CCTools : public CommandInterface
{

private:
    int _CC_RST_PIN, _CC_BSL_PIN, _BSL_PIN_MODE;

public:
    uint32_t currentAddr = 0x00000000;
    static const int TRANSFER_SIZE = 248;
    byte emptyPacket[TRANSFER_SIZE];
    bool ledState;
    bool bslActive;

    zbInfoStruct chip;

    CCTools(Stream &serial) : CommandInterface(serial)
    {
        memset(emptyPacket, 0xFF, TRANSFER_SIZE);
        chip.fwRev = 0;
        chip.maintrel = 0;
        chip.minorrel = 0;
        chip.majorrel = 0;
        chip.product = 0;
        chip.transportrev = 0;
        chip.hwRev = "";
        ledState = 0;
        bslActive = 0;
    }

    bool begin(int CC_RST_PIN, int CC_BSL_PIN, int BSL_PIN_MODE = 0);
    void enterBSL();
    void restart();
    void routerRejoin();
    bool detectChipInfo();
    bool eraseFlash();
    bool beginFlash(uint32_t startAddr, int totalSize);
    void processFlash(byte *data, int size);
    bool checkFirmwareVersion();
    bool ledToggle();
    bool nvram_reset(void (*logFunction)(const String&));
};

#endif // CCTools_DETECT_H