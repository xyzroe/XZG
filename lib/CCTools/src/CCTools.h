#ifndef CCTools_DETECT_H
#define CCTools_DETECT_H

#include <Arduino.h>

class CommandInterface
{
protected:
    Stream &_stream;

    static const uint8_t ACK_BYTE = 0xCC;
    static const uint8_t NACK_BYTE = 0x33;
    static const uint8_t COMMAND_RET_SUCCESS = 0x40;

    static const uint32_t ICEPICK_DEVICE_ID = 0x50001318;
    static const uint32_t FCFG_USER_ID = 0x50001294;
    static const uint32_t PRCM_RAMHWOPT = 0x40082250;
    static const uint32_t FLASH_SIZE = 0x4003002C;
    static const uint32_t PROTO_MASK_BLE = 0x01;
    static const uint32_t PROTO_MASK_IEEE = 0x04;
    static const uint32_t PROTO_MASK_BOTH = 0x05;

    static const uint32_t flash_start_addr = 0x00000000;

    static const uint32_t addr_ieee_address_primary = 0x500012F0;
    static const uint32_t ccfg_len = 88;
    static const uint32_t ieee_address_secondary_offset = 0x20;
    static const uint32_t bootloader_dis_offset = 0x30;

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

    const char *_getChipIdString(uint32_t chipId)
    {
        switch (chipId)
        {
        case 0xb964:
            return "CC2538";
        case 0xb965:
            return "CC2538";
        case 0xf000:
            return "CC2652";
        default:
            return "Unknown";
        }
    }

    String _getPackage(byte user_id)
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

    CommandInterface(Stream &serial);

    bool _sendSynch();
    bool _wait_for_ack(unsigned long timeout);
    uint32_t _cmdGetChipId();
    byte *_receivePacket();
    byte *_cmdGetStatus();
    bool _checkLastCmd();
    void _sendAck();
    void _sendNAck();
    void _eraseFlash();
    bool _ping();
    void _encodeAddr(unsigned long addr, byte encodedAddr[4]);
    unsigned long _decodeAddr(byte byte0, byte byte1, byte byte2, byte byte3);
    byte _calcChecks(byte cmd, unsigned long addr, unsigned long size);
    byte *_cmdMemRead(uint32_t address);
};

class CCTools_detect : public CommandInterface
{
private:
    int _CC_RST_PIN, _CC_BSL_PIN, _BSL_MODE;

    void _enterBSLMode();

public:
    CCTools_detect(Stream &serial);
    bool begin(int CC_RST_PIN, int CC_BSL_PIN, int BSL_MODE = 0);
    bool eraseFlash();
    bool ping();
    String detectChipInfo();
};

#endif // CCTools_DETECT_H