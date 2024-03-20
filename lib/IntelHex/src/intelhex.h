#ifndef INTELHEX_H
#define INTELHEX_H

#include <Arduino.h>
#include "LittleFS.h"

#define ELEMENTCOUNT(x) (sizeof(x) / sizeof(x[0]))

class IntelHex
{
public:
    IntelHex(const char *filename);
    ~IntelHex();

    bool parse(void (*preCallback)(), void (*parseCallback)(uint32_t address, uint8_t len, uint8_t *data, size_t currentPosition, size_t totalSize), void (*postCallback)());
    void setFileValidated(bool value) { _file_validated = value; }

    bool fileParsed() const { return _file_parsed; }
    bool fileValidated() const { return _file_validated; }

    bool bslActive() const { return _bsl_valid; }
    int bslPin() const { return _bsl_pin; }
    bool bslLevel() const { return _bsl_level - 1; } // 0 error, 1 low, 2 high
    bool bslAddr() const { return _bsl_addr; }       // 0 - all seriers, 1 - P/R 7 series


    bool validateChecksum(); // Построчная валидация контрольной суммы
    bool checkBSLConfiguration(); // Проверка конфигурации BSL


private:
    const char *_filename;
    File _file;
    bool _file_parsed = false;
    bool _file_validated = false;
    bool _file_last_line = false;
    uint32_t _offset_high;

    bool _bsl_bootloader_enbl = false;
    bool _bsl_bl_enbl = false;
    int _bsl_level = 0;
    int _bsl_pin = 0;
    bool _bsl_valid = false;
    int _bsl_addr = 0;

    size_t _totalSize = 0;

    // const uint32_t CCFG_ADDRESS = 0x057FD8; // all others
    const uint32_t ALL_CHIP_ADDRESS = 0x057FD8;
    const uint32_t P7_CHIP_ADDRESS = 0x0AFFD8;
    const uint32_t CCFG_ADDRESS[2] = {ALL_CHIP_ADDRESS, P7_CHIP_ADDRESS};

#define ALL_CHIP_ID 0
#define P7_CHIP_ID 1

    // const uint32_t CCFG_ADDRESS = 0x0AFFD8; //CC2652R7 and CC1352P7

    /*
    Parsing line: :020000025000AC ordinary so 0x057FD8
    Parsing line: :02000002A0005C but in 2652R7 so maybe 0x0AFFD8
    */

    const uint32_t BOOTLOADER_ENABLE = 0xC5; //(Bootloader enable. SET_CCFG_BL_CONFIG_BOOTLOADER_ENABLE in CC13xx/CCToolsware)

    const uint32_t BL_LEVEL_LOW = 0xFE;  //(Active low. SET_CCFG_BL_CONFIG_BL_LEVEL in CC13xx/CCToolsware)
    const uint32_t BL_LEVEL_HIGH = 0xFF; // ? NEED to check!

    const uint32_t BL_ENABLE = 0xC5; // (Enable "failure analysis". SET_CCFG_BL_CONFIG_BL_ENABLE in CC13xx/CCToolsware)

    bool open();
    void close();
    bool _munchLine(void (*parseCallback)(uint32_t address, uint8_t len, uint8_t *data, size_t currentPosition, size_t totalSize));
    bool _checkBSLconfig(uint32_t address, uint8_t len, uint8_t *data);
};

#endif
