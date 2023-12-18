#include "intelhex.h"
#include "config.h"

/*
 * IntelHex Arduino Library
 *
 * Description:
 * This library is designed to read and parse Intel Hex format files from the LittleFS file system on ESP8266 and ESP32.
 *
 * License:
 * MIT License
 *
 * Author:
 * OpenAI's ChatGPT with contributions from xyzroe
 */
/*

If you want library to control LittleFS you need to uncomment some lines.


TO-DO List

1. Line length check
2. Error if not IntelHEX line after start till end

*/

IntelHex::IntelHex(const char *filename) : _filename(filename) {}

IntelHex::~IntelHex()
{
    if (_file)
    {
        _file.close();
    }
}

bool IntelHex::open()
{
    // if (!LittleFS.begin()) {
    //     Serial.println("Could not mount file system");
    //     return false;
    // }
    _file = LittleFS.open(_filename, "r");
    return _file;
}

void IntelHex::close()
{
    _file.close();
    // LittleFS.end();
}

bool IntelHex::parse(void (*preCallback)(), void (*parseCallback)(uint32_t address, uint8_t len, uint8_t *data), void (*postCallback)())
{
    if (!open())
    {
        return false;
    }

    preCallback();

    bool status = true;

    while (status == true && _file.available() && _file_last_line == false)
    {
        status = _munchLine(parseCallback);
    }

    if (_file_last_line)
    {
        DEBUG_PRINTLN("HEX file last line found");
        _file_parsed = true;
    }
    else
    {
        DEBUG_PRINTLN("HEX file last line NOT found");
        status = false;
    }

    postCallback();

    close();
    return status;
}

bool IntelHex::_munchLine(void (*parseCallback)(uint32_t address, uint8_t len, uint8_t *data))
{
    String line = _file.readStringUntil('\n');
    line.trim();

    // DEBUG_PRINTLN("Parsing line: " + line); // Print each processed line
    // delay(1);

    if (line.length() == 0 || line[0] != ':')
    {
        return true; // Continue parsing
    }

    uint8_t sum = 0;
    for (int i = 1; i < line.length(); i += 2)
    {
        sum += strtol(line.substring(i, i + 2).c_str(), nullptr, 16);
    }

    if (sum != 0)
    {
        DEBUG_PRINTLN("Checksum line error");
        return false;
    }

    if (line == ":00000001FF")
    { // look like Intel HEX file end line
        DEBUG_PRINTLN("File last line found");
        _file_last_line = true;
        return true;
    }

    uint8_t len = strtol(line.substring(1, 3).c_str(), nullptr, 16);
    uint16_t offset_low = strtol(line.substring(3, 7).c_str(), nullptr, 16);
    uint8_t recordType = strtol(line.substring(7, 9).c_str(), nullptr, 16);

    // DEBUG_PRINTLN("Record Type: " + String(recordType, HEX) + ", Offset Low: " + String(offset_low, HEX) + ", Length: " + String(len, HEX)); // Print record info

    uint8_t data[255];
    for (uint8_t i = 0; i < len; i++)
    {
        data[i] = strtol(line.substring(9 + i * 2, 11 + i * 2).c_str(), nullptr, 16);
    }

    uint32_t address = offset_low;
    // https://jimmywongiot.com/2021/04/20/format-of-intelhex/
    if (recordType == 4)
    {
        _offset_high = ((uint32_t)strtol(line.substring(9, 13).c_str(), nullptr, 16)) << 16; // because 4
    }
    else if (recordType == 2)
    {
        _offset_high = ((uint32_t)strtol(line.substring(9, 13).c_str(), nullptr, 16)) << 4; // because 2
    }

    address += _offset_high;

    if (!_bsl_valid)
    {
        _bsl_valid = _checkBSLconfig(address, len, data);
    }
    parseCallback(address, len, data);

    return true;
}

bool IntelHex::_checkBSLconfig(uint32_t address, uint8_t len, uint8_t *data)
{
    // Check if CCFG is within the buffer
     //DEBUG_PRINTLN(" ");
     //DEBUG_PRINT(ELEMENTCOUNT(CCFG_ADDRESS));
     //DEBUG_PRINT(" ");

    // DEBUG_PRINTLN(ESP.getFreeHeap() / 1024);
    delay(5);
    for (int i = 0; i < ELEMENTCOUNT(CCFG_ADDRESS); i++)
    {
         //DEBUG_PRINT(CCFG_ADDRESS[i]);
         //DEBUG_PRINT(" ");
        if (address <= CCFG_ADDRESS[i] && address + len > CCFG_ADDRESS[i] + 4)
        {
            DEBUG_PRINTLN(" ");
            DEBUG_PRINTLN("CCFG_ADDRESS[" + String(i) + "] in range");

            if (data[CCFG_ADDRESS[i] - address + 3] == BOOTLOADER_ENABLE)
            {
                DEBUG_PRINTLN("Bootloader enabled");
                _bsl_bootloader_enbl = true;
            }

            if (data[CCFG_ADDRESS[i] - address + 0] == BL_ENABLE)
            {
                DEBUG_PRINTLN("'failure analysis' enabled");
                _bsl_bl_enbl = true;
            }

            if (data[CCFG_ADDRESS[i] - address + 2] == BL_LEVEL_LOW)
            {
                DEBUG_PRINTLN("BSL low level");
                _bsl_level = 1;
            }
            else if (data[CCFG_ADDRESS[i] - address + 2] == BL_LEVEL_HIGH)
            {
                DEBUG_PRINTLN("BSL high level");
                _bsl_level = 2;
            }
            else
            {
                DEBUG_PRINTLN("BSL level UNKNOWN. Error!");
                _bsl_level = 0;
            }

            // Pin in HEX converts to DEC number
            _bsl_pin = data[CCFG_ADDRESS[i] - address + 1];
            DEBUG_PRINTLN("BSL pin - " + String(_bsl_pin));

            if (_bsl_bootloader_enbl && _bsl_bl_enbl && (_bsl_level > 0) && (_bsl_pin > 0))
            {
                DEBUG_PRINTLN("BSL valid!");
                _bsl_addr = i;
                return true;
            }
        }
    }
    return false;
}