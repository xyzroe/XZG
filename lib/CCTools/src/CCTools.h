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

    const char *_getChipDescription(uint32_t chip_id, uint32_t wafer_id, uint8_t pg_rev)
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
        else if (chip_id == 0x3202 && wafer_id == 0xBB41 && pg_rev == 0x3)
        {
            return "CC2652P";
        }
        else
        {
            static char unknownDescription[50];
            sprintf(unknownDescription, "Unknown (C: %X, W: %X, P: %X)",
                    chip_id, wafer_id, pg_rev);
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
};

#endif // CCTools_DETECT_H