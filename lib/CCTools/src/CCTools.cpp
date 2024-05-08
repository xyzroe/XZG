#include <Arduino.h>
#include "CCTools.h"
#include <HTTPClient.h>
#include <WiFi.h>

#ifndef DEBUG_PRINT
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(String(x))
#define DEBUG_PRINTLN(x) Serial.println(String(x))
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
#endif

namespace
{
    uint16_t findTableEnd(uint16_t value)
    {
        for (int i = 0; i < sizeof(NWK_NVID_TABLES) / sizeof(NWK_NVID_TABLES[0]); ++i)
        {
            if (value == NWK_NVID_TABLES[i].first)
            {
                return NWK_NVID_TABLES[i].second;
            }
        }
        return false;
    }

}

CommandInterface::CommandInterface(Stream &serial) : _stream(serial) {}

void CommandInterface::_cleanBuffer()
{
    while (_stream.available())
    { // clear buffer
        _stream.read();
    }
}

bool CommandInterface::_sendSynch()
{
    const u_int32_t cmd = 0x55;

    _stream.flush();

    _stream.write(cmd); // # send U
    _stream.write(cmd); // # send U
    return _wait_for_ack(2);
}

bool CommandInterface::_wait_for_ack(unsigned long timeout = 1)
{
    unsigned long startMillis = millis();
    while (millis() - startMillis < timeout * 1000)
    {
        if (_stream.available() >= 1)
        {
            uint8_t received = _stream.read();
            if (received == ACK_BYTE)
            {
                // Serial.println("ACK received");
                return true;
            }
            else if (received == NACK_BYTE)
            {
                // Serial.println("NACK received");
                return false;
            }
        }
    }
    Serial.println("Timeout waiting for ACK/NACK");
    return false;
}

byte *CommandInterface::_receive_SRSP(unsigned long timeout = 500)
{
    unsigned long startMillis = millis();
    while (millis() - startMillis < timeout)
    {
        if (_stream.available() >= 1)
        {
            if (_stream.read() == cmdFrameStart)
            {
                uint8_t size = _stream.read();
                byte *data = new byte[size + 3];
                if (!_stream.readBytes(data, size + 3))
                {
                    delete[] data;
                    return nullptr;
                }
                else
                {
                    // DEBUG_PRINT("Success ");
                    return data;
                }
            }
            else
            {
                DEBUG_PRINT("Wrong answer ");
                return nullptr;
            }
        }
    }
    DEBUG_PRINT("Timeout ");
    return nullptr;
}

uint32_t CommandInterface::_cmdGetChipId()
{
    const u_int32_t cmd = 0x28;
    const u_int32_t lng = 3;

    _stream.write(lng); //  # send size
    _stream.write(cmd); //  # send checksum
    _stream.write(cmd); //  # send data
    if (_wait_for_ack())
    {
        // 4 byte answ, the 2 LSB hold chip ID
        byte *version = _receivePacket();
        // Serial.println("size " + String(sizeof(version)));
        if (_checkLastCmd())
        {

            uint32_t value = 0;
            value |= uint32_t(version[3]) << 0; // Least significant byte
            value |= uint32_t(version[2]) << 8;
            value |= uint32_t(version[1]) << 16;
            value |= uint32_t(version[0]) << 24; // Most significant byte
            // Serial.print("ChipId ");
            // Serial.println(value, HEX);

            if (sizeof(version) != 4)
            {
                Serial.println("Unreasonable chip. Looks upper"); // repr(version) ?
                return uint32_t(0);
            }

            uint32_t chip_id = (version[0] << 8) | (version[1]);

            return chip_id;
        }
    }
    return uint32_t(0);
}

byte *CommandInterface::_cmdGetStatus()
{
    const u_int32_t cmd = 0x23;
    const u_int32_t lng = 3;

    _stream.write(lng); //  # send size
    _stream.write(cmd); //  # send checksum
    _stream.write(cmd); //  # send data

    // mdebug(10, "*** GetStatus command (0x23)")
    if (_wait_for_ack())
    {
        byte *stat = _receivePacket();
        return stat;
    }
    Serial.print("Error _cmdGetStatus");
    return nullptr;
}

bool CommandInterface::_checkLastCmd()
{

    byte *stat = _cmdGetStatus();
    if (stat == nullptr)
    {
        Serial.println("No response from target on status request.");
        Serial.println("(Did you disable the bootloader?)");
        return 0;
    }
    else
    {
        if (stat[0] == COMMAND_RET_SUCCESS)
        {
            // Serial.println("Command Successful");
            return 1;
        }
        else
        {
            const char *stat_str = _getStatusString(stat[0]);
            if (stat_str == "Unknown")
            {
                Serial.println("Warning: unrecognized status returned 0x" + String(stat[0]));
            }
            else
            {
                Serial.println("Target returned: 0x" + String(stat[0]) + " " + String(stat_str));
            }
            return 0;
        }
    }
    return 0;
}

void CommandInterface::_sendAck()
{
    const u_int32_t cmd1 = 0x00;
    const u_int32_t cmd2 = 0xCC;
    _stream.write(cmd1);
    _stream.write(cmd2);
}

void CommandInterface::_sendNAck()
{
    const u_int32_t cmd1 = 0x00;
    const u_int32_t cmd2 = 0x33;
    _stream.write(cmd1);
    _stream.write(cmd2);
}

bool CommandInterface::_eraseFlash()
{
    const u_int32_t cmd = 0x2C;
    const u_int32_t lng = 3;

    _stream.write(lng); //  # send size
    _stream.write(cmd); //  # send checksum
    _stream.write(cmd); //  # send data

    if (_wait_for_ack())
    {
        if (_checkLastCmd())
        {
            return true;
        }
    }
    return false;
}

bool CommandInterface::_cmdDownload(uint32_t address, unsigned long size)
{
    const u_int32_t cmd = 0x21;
    const u_int32_t lng = 11;

    if (size % 4 != 0)
    {
        // If size is not a multiple of 4, handle the error
        Serial.print("Invalid data size: ");
        Serial.print(size);
        Serial.println(". Size must be a multiple of 4.");
        return false;
    }

    byte addrBytes[4];
    _encodeAddr(address, addrBytes);

    byte sizeBytes[4];
    _encodeAddr(size, sizeBytes);

    _stream.write(lng);                             // # send length
    _stream.write(_calcChecks(cmd, address, size)); // # send checksum
    _stream.write(cmd);                             // # send cmd
    _stream.write(addrBytes[0]);                    // # send addr
    _stream.write(addrBytes[1]);                    // # send addr
    _stream.write(addrBytes[2]);                    // # send addr
    _stream.write(addrBytes[3]);                    // # send addr
    _stream.write(sizeBytes[0]);                    // # send size
    _stream.write(sizeBytes[1]);                    // # send size
    _stream.write(sizeBytes[2]);                    // # send size
    _stream.write(sizeBytes[3]);                    // # send size

    // Serial.println("*** Mem Read (0x2A)");
    if (_wait_for_ack())
    {
        byte *data = _receivePacket();
        if (_checkLastCmd())
        {
            return true;
        }
    }
    return false;
}

bool CommandInterface::_cmdSendData(byte *data, unsigned int dataSize)
{
    const uint32_t cmd = 0x24;
    const uint32_t maxDataSize = 252; // define the maximum allowable data size

    // Check if data size exceeds maximum limit
    if (dataSize > maxDataSize)
    {
        Serial.print("Data size too large: ");
        Serial.print(dataSize);
        Serial.println(". Maximum size allowed is 252 bytes.");
        return false;
    }

    uint32_t lng = dataSize + 3;

    // Calculate checksum
    byte checksum = cmd;
    for (unsigned int i = 0; i < dataSize; i++)
    {
        checksum += data[i];
    }
    checksum &= 0xFF; // ensure checksum is within a byte

    _stream.write(lng);      // send length
    _stream.write(checksum); // send checksum
    _stream.write(cmd);      // send cmd
    for (unsigned int i = 0; i < dataSize; i++)
    {
        _stream.write(data[i]); // send data
    }

    // Optionally print debug information to the serial monitor
    // Serial.println("*** Send Data (0x24)");

    // Assume _wait_for_ack() and _checkLastCmd() are implemented similarly to your previous method
    if (_wait_for_ack())
    {
        if (_checkLastCmd())
        {
            return true;
        }
    }
    return false;
}

bool CommandInterface::_ping()
{
    const u_int32_t cmd1 = 0x20;
    _stream.write(cmd1);
    return _wait_for_ack(1);
}

byte *CommandInterface::_cmdMemRead(uint32_t address)
{
    const u_int32_t cmd = 0x2A;
    const u_int32_t lng = 9;

    byte addrBytes[4];
    _encodeAddr(address, addrBytes);

    _stream.write(lng);                          // # send length
    _stream.write(_calcChecks(cmd, address, 2)); // # send checksum
    _stream.write(cmd);                          // # send cmd
    _stream.write(addrBytes[0]);                 // # send addr
    _stream.write(addrBytes[1]);                 // # send addr
    _stream.write(addrBytes[2]);                 // # send addr
    _stream.write(addrBytes[3]);                 // # send addr
    _stream.write(1);                            // # send width, 4 bytes
    _stream.write(1);                            // # send number of reads

    // Serial.println("*** Mem Read (0x2A)");
    if (_wait_for_ack())
    {
        byte *data = _receivePacket();
        if (_checkLastCmd())
        {
            return data;
        }
    }
    return 0;
}

byte *CommandInterface::_receivePacket()
{
    byte got[2];
    // Read the initial 2 bytes which contain size and checksum
    // The _read function should be defined elsewhere to read bytes from a communication interface.
    _stream.readBytes(got, 2);

    byte size = got[0];              // rcv size
    byte chks = got[1];              // rcv checksum
    byte *data = new byte[size - 2]; // Allocate buffer for the data

    // Now read the rest of the packet
    if (!_stream.readBytes(data, size - 2))
    {
        // Handle read error
        delete[] data; // Remember to free the memory if an error occurs
        return nullptr;
    }

    // Debugging output, might use Serial.print in Arduino
    // Serial.print(F("*** received "));
    // Serial.print(size, HEX);
    // Serial.println(F(" bytes"));

    // Calculate checksum
    byte calculatedChks = 0;
    for (int i = 0; i < size - 2; i++)
    {
        calculatedChks += data[i];
    }
    calculatedChks &= 0xFF;

    // Check the checksum
    if (chks == calculatedChks)
    {
        _sendAck(); // This function needs to be implemented
        return data;
    }
    else
    {
        _sendNAck();   // This function needs to be implemented
        delete[] data; // Free the memory
        // Handle checksum error
        // You could set an error flag or return a null pointer.
        return nullptr;
    }
}

void CommandInterface::_encodeAddr(unsigned long addr, byte encodedAddr[4])
{
    encodedAddr[3] = (byte)((addr >> 0) & 0xFF);
    encodedAddr[2] = (byte)((addr >> 8) & 0xFF);
    encodedAddr[1] = (byte)((addr >> 16) & 0xFF);
    encodedAddr[0] = (byte)((addr >> 24) & 0xFF);
}

unsigned long CommandInterface::_decodeAddr(byte byte0, byte byte1, byte byte2, byte byte3)
{
    return ((unsigned long)byte3 << 24) | ((unsigned long)byte2 << 16) | ((unsigned long)byte1 << 8) | byte0;
}

byte CommandInterface::_calcChecks(byte cmd, unsigned long addr, unsigned long size)
{
    byte addrBytes[4];
    byte sizeBytes[4];
    _encodeAddr(addr, addrBytes);
    _encodeAddr(size, sizeBytes);

    unsigned long sum = 0;
    for (int i = 0; i < 4; i++)
    {
        sum += addrBytes[i];
        sum += sizeBytes[i];
    }
    sum += cmd;

    return (byte)(sum & 0xFF);
}

bool CommandInterface::_ledToggle(bool ledState)
{
    _cleanBuffer();
    if (ledState == 0)
    {
        _stream.write(zigLed1On, sizeof(zigLed1On));
    }
    else
    {
        _stream.write(zigLed1Off, sizeof(zigLed1Off));
    }
    _stream.flush();
    delay(400);
    for (uint8_t i = 0; i < 5; i++)
    {
        if (_stream.read() != cmdFrameStart)
        {                   // check for packet start
            _stream.read(); // skip
        }
        else
        {
            for (uint8_t i = 1; i < 4; i++)
            {
                if (_stream.read() == cmdLedResp[i])
                { // check if resp ok
                    return true;
                }
            }
        }
    }
    return false;
}

bool CommandInterface::_nvram_osal_delete(uint16_t nvid)
{
    // DEBUG_PRINT("Checking OsalNvIds ID: ");
    // Serial.print(nvid, HEX);
    // DEBUG_PRINT(" - ");

    const uint8_t cmd1 = 0x21;
    uint8_t cmd2 = 0x13;
    uint8_t lng = 0x02;

    uint8_t highByte = (nvid >> 8) & 0xFF; // Старший байт (верхние 8 бит)
    uint8_t lowByte = nvid & 0xFF;         // Младший байт (нижние 8 бит)

    uint8_t fcs = 0;

    _stream.write(cmdFrameStart);

    _stream.write(lng);
    fcs ^= lng;
    _stream.write(cmd1);
    fcs ^= cmd1;
    _stream.write(cmd2);
    fcs ^= cmd2;
    _stream.write(lowByte);
    fcs ^= lowByte;
    _stream.write(highByte);
    fcs ^= highByte;

    _stream.write(fcs);

    _stream.flush();

    // Serial.println("");
    // Serial.println("> " + String(cmd1, HEX) + " " + String(cmd2, HEX) + " " + String(lowByte, HEX) + " " + String(highByte, HEX) + " " + String(fcs, HEX));

    std::unique_ptr<byte[]> data(_receive_SRSP());
    if (!data)
    {
        return false;
    }
    // Serial.println("< " + String(data[0], HEX) + " " + String(data[1], HEX) + " " + String(data[2], HEX) + " " + String(data[3], HEX) + " " + String(data[4], HEX));

    if (data[2] > 0 || data[3] > 0)
    {
        /*
        DEBUG_PRINT("* Deleting OsalNvIds ID: ");
        Serial.print(nvid, HEX);
        DEBUG_PRINT(" - ");
        */

        fcs = 0;
        cmd2 = 0x12;
        lng = 0x04;

        _stream.write(cmdFrameStart);

        _stream.write(lng);
        fcs ^= lng;
        _stream.write(cmd1);
        fcs ^= cmd1;
        _stream.write(cmd2);
        fcs ^= cmd2;
        _stream.write(lowByte);
        fcs ^= lowByte;
        _stream.write(highByte);
        fcs ^= highByte;
        _stream.write(data[2]);
        fcs ^= data[2];
        _stream.write(data[3]);
        fcs ^= data[3];

        _stream.write(fcs);

        _stream.flush();

        // Serial.println("");
        // Serial.println("> " + String(cmd1, HEX) + " " + String(cmd2, HEX) + " " + String(lowByte, HEX) + " " + String(highByte, HEX) + " " + String(data[2], HEX) + " " + String(data[3], HEX) + " " + String(fcs, HEX));

        std::unique_ptr<byte[]> data(_receive_SRSP());
        if (!data)
        {
            return false;
        }
        // Serial.println("< " + String(data[0], HEX) + " " + String(data[1], HEX) + " " + String(data[2], HEX) + " " + String(data[3], HEX));
    }
    return true;
}

bool CommandInterface::_nvram_ex_delete(uint16_t nvid, uint16_t subID)
{
    /*
    DEBUG_PRINT("Deleting ExNvIds sub ID: ");

    Serial.print(nvid, HEX);
    DEBUG_PRINT(" ");
    Serial.print(subID, HEX);
    DEBUG_PRINT(" - ");
    */
    const uint8_t cmd1 = 0x21;
    uint8_t cmd2 = 0x31;
    uint8_t lng = 0x05;
    const uint8_t sysID = 1;

    uint8_t highByte = (nvid >> 8) & 0xFF;
    uint8_t lowByte = nvid & 0xFF;

    uint8_t subIDhighByte = (subID >> 8) & 0xFF;
    uint8_t subIDlowByte = subID & 0xFF;

    uint8_t fcs = 0;

    _stream.write(cmdFrameStart);

    _stream.write(lng);
    fcs ^= lng;
    _stream.write(cmd1);
    fcs ^= cmd1;
    _stream.write(cmd2);
    fcs ^= cmd2;
    _stream.write(sysID);
    fcs ^= sysID;
    _stream.write(lowByte);
    fcs ^= lowByte;
    _stream.write(highByte);
    fcs ^= highByte;
    _stream.write(subIDlowByte);
    fcs ^= subIDlowByte;
    _stream.write(subIDhighByte);
    fcs ^= subIDhighByte;

    _stream.write(fcs);

    _stream.flush();

    std::unique_ptr<byte[]> data(_receive_SRSP());
    if (!data)
    {
        return false;
    }
    // Serial.println(String(data[2], HEX));

    if (data[2] == 0x0A)
    { // error
        return false;
    }

    return true;
}

CommandInterface::zbInfoStruct CommandInterface::_checkFwVer()
{
    zbInfoStruct chip;
    for (uint8_t i = 0; i < 10; i++)
    {
        if (_stream.read() != cmdFrameStart || _stream.read() != 0x0a || _stream.read() != 0x61 || _stream.read() != cmdVer2)
        { // check for packet start
            _cleanBuffer();
            _stream.write(cmdSysVersion, sizeof(cmdSysVersion));
            _stream.flush();
            delay(100);
        }
        else
        {
            const uint8_t zbVerLen = 11;
            byte zbVerBuf[zbVerLen];
            for (uint8_t i = 0; i < zbVerLen; i++)
            {
                zbVerBuf[i] = _stream.read();
            }
            chip.fwRev = zbVerBuf[5] | (zbVerBuf[6] << 8) | (zbVerBuf[7] << 16) | (zbVerBuf[8] << 24);
            chip.maintrel = zbVerBuf[4];
            chip.minorrel = zbVerBuf[3];
            chip.majorrel = zbVerBuf[2];
            chip.product = zbVerBuf[1];
            chip.transportrev = zbVerBuf[0];
            _cleanBuffer();

            DEBUG_PRINTLN("ZB v: " + String(chip.fwRev) + " Main: " + chip.maintrel + " Min: " + chip.minorrel + " Maj: " + chip.majorrel + " T: " + chip.transportrev + " P: " + chip.product);

            return chip;
        }
    }
    chip.fwRev = 0;
    return chip;
}

bool CCTools::begin(int CC_RST_PIN, int CC_BSL_PIN, int BSL_PIN_MODE)
{
    _CC_RST_PIN = CC_RST_PIN;
    _CC_BSL_PIN = CC_BSL_PIN;
    _BSL_PIN_MODE = BSL_PIN_MODE;

    pinMode(_CC_RST_PIN, OUTPUT);
    digitalWrite(_CC_RST_PIN, HIGH);
    pinMode(_CC_BSL_PIN, OUTPUT);
    digitalWrite(_CC_BSL_PIN, HIGH);

    enterBSL();

    //_cleanBuffer();

    if (!_sendSynch())
    {
        // DEBUG_PRINT("NO BEGIN ANSWER");
        return false;
    }

    return true;
}

void CCTools::enterBSL()
{
    // Other modes BSL_PIN_MODE can be added later
    if (_BSL_PIN_MODE == 0)
    {
        digitalWrite(_CC_RST_PIN, LOW);
        digitalWrite(_CC_BSL_PIN, LOW);
        // DEBUG_PRINTLN(F("Zigbee RST & BSL pin ON"));
        delay(50);
        digitalWrite(_CC_RST_PIN, HIGH);
        // DEBUG_PRINTLN(F("Zigbee RST pin OFF"));
        delay(500);
        digitalWrite(_CC_BSL_PIN, HIGH);
        // DEBUG_PRINTLN(F("Zigbee BSL pin OFF"));
        delay(500);
    }
    bslActive = 1;
}

void CCTools::restart()
{
    digitalWrite(_CC_RST_PIN, LOW);
    // DEBUG_PRINTLN(F("Zigbee RST pin ON"));
    delay(50);
    digitalWrite(_CC_RST_PIN, HIGH);
    // DEBUG_PRINTLN(F("Zigbee RST pin OFF"));
    delay(500);
    bslActive = 0;
}

void CCTools::routerRejoin()
{
    digitalWrite(_CC_BSL_PIN, LOW);
    // DEBUG_PRINTLN(F("ZB BSL pin ON"));
    delay(250);
    digitalWrite(_CC_BSL_PIN, HIGH);
    // DEBUG_PRINTLN(F("ZB BSL pin OFF"));
    delay(500);
}

bool CCTools::detectChipInfo()
{
    if (!bslActive)
    {
        enterBSL();
        if (!_sendSynch())
        {
            return false;
        }
    }

    uint32_t chip_id = _cmdGetChipId();

    // Serial.println(chip_id, HEX);

    byte *device_id = _cmdMemRead(ICEPICK_DEVICE_ID);
    // Serial.println(sizeof(device_id));

    uint32_t wafer_id = (((device_id[3] & 0x0F) << 16) +
                         (device_id[2] << 8) +
                         (device_id[1] & 0xF0)) >>
                        4;
    uint32_t pg_rev = (device_id[3] & 0xF0) >> 4;

    // We can now detect the exact device

    // Serial.print("wafer_id: ");
    // Serial.println(wafer_id, HEX);
    // Serial.print("pg_rev: ");
    // Serial.println(pg_rev, HEX);

    byte *user_id = _cmdMemRead(FCFG_USER_ID);

    // Serial.println("Package: " + _getPackageString(user_id[2]));

    byte protocols = user_id[1] >> 4;
    // Serial.print("protocols: ");
    // Serial.println(protocols, HEX);

    byte *flash_size = _cmdMemRead(FLASH_SIZE);
    // Serial.print("flash_size: ");
    // Serial.println(flash_size[0], HEX);

    // byte *ram_size = _cmdMemRead(PRCM_RAMHWOPT);
    // Serial.print("ram_size: ");
    // Serial.println(ram_size[0], HEX);

    byte *ieee_b1 = _cmdMemRead(addr_ieee_address_primary + 4);
    byte *ieee_b2 = _cmdMemRead(addr_ieee_address_primary);

    if (ieee_b1 == nullptr || ieee_b2 == nullptr)
    {
        Serial.println("Error read IEEE");
        return false;
    }

    String ieeeAddr;

    for (int i = 3; i >= 0; --i)
    {
        ieeeAddr += (ieee_b1[i] < 16 ? "0" : "") + String(ieee_b1[i], HEX) + ":";
    }
    for (int i = 3; i >= 0; --i)
    {
        ieeeAddr += (ieee_b2[i] < 16 ? "0" : "") + String(ieee_b2[i], HEX) + (i > 0 ? ":" : "");
    }
    ieeeAddr.toUpperCase();

    chip.ieee = ieeeAddr;

    delete[] ieee_b1;
    delete[] ieee_b2;

    String chip_str;
    if (protocols & PROTO_MASK_IEEE == PROTO_MASK_IEEE)
    {
        uint32_t test = 360372;
        // Serial.print(test, HEX);
        byte *b_val = _cmdMemRead(test);

        chip.hwRev = _getChipDescription(chip_id, wafer_id, pg_rev, b_val[1]);
        int page_size = 4096;
        if (chip.hwRev.indexOf("P7"))
        {
            page_size = page_size * 2;
        }
        chip.flashSize = flash_size[0] * page_size;

        test = chip.flashSize - 88 + 0xC;
        Serial.print(test, HEX);
        b_val = _cmdMemRead(test);
        Serial.print(" MODE_CONF: ");
        Serial.print(b_val[0], HEX);
        Serial.print(b_val[1], HEX);
        Serial.print(b_val[2], HEX);
        Serial.println(b_val[3], HEX);

        uint32_t bsl_adr = chip.flashSize - 88 + 0x30;
        Serial.print(bsl_adr, HEX);
        byte *bsl_val = _cmdMemRead(bsl_adr);
        Serial.print(" bsl_val: ");
        Serial.print(bsl_val[0], HEX);
        Serial.print(bsl_val[1], HEX);
        Serial.print(bsl_val[2], HEX);
        Serial.println(bsl_val[3], HEX);

        return true;
    }
    return false;
}

bool CCTools::eraseFlash()
{
    if (!bslActive)
    {
        enterBSL();
        if (!_sendSynch())
        {
            return false;
        }
    }

    bool result = _eraseFlash();

    return result;
}

bool CCTools::beginFlash(uint32_t startAddr, int totalSize)
{
    this->currentAddr = startAddr;
    return _cmdDownload(startAddr, totalSize);
}

void CCTools::processFlash(byte *data, int size)
{
    if (memcmp(data, this->emptyPacket, size) != 0)
    {
        if (!_cmdSendData(data, size))
        {
            return;
        }
    }
    this->currentAddr += size;
}

bool CCTools::checkFirmwareVersion()
{
    if (bslActive)
    {
        restart();
    }
    zbInfoStruct temp = _checkFwVer();
    chip.fwRev = temp.fwRev;
    chip.maintrel = temp.maintrel;
    chip.minorrel = temp.minorrel;
    chip.majorrel = temp.majorrel;
    chip.product = temp.product;
    chip.transportrev = temp.transportrev;

    if (chip.fwRev > 0)
    {
        return true;
    }
    return false;
}

bool CCTools::ledToggle()
{
    if (bslActive)
    {
        restart();
    }

    if (_ledToggle(ledState))
    {
        ledState = !ledState;
        return true;
    }
    return false;
}

bool CCTools::nvram_reset(void (*logFunction)(String))
{
    bool success = true;

    // The legacy items are shared by all Z-Stack versions
    bool tableFinish = false;
    for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i)
    {
        uint16_t table_end = findTableEnd(keys[i]);
        if (table_end != 0)
        {
            tableFinish = true;
            uint16_t table_start = keys[i];
            for (uint16_t id = table_start; id <= table_end; id++)
            {
                String msg = "Nv [T] ID: " + String(id, HEX);
                logFunction(msg);

                if (!_nvram_osal_delete(id))
                {
                    success = false;
                    break;
                }
            }
        }
        else
        {
            if (!tableFinish)
            {
                String msg = "Nv [K] ID:" + String(keys[i], HEX);
                logFunction(msg);

                if (!_nvram_osal_delete(keys[i]))
                {
                    success = false;
                }
            }
            else
            {
                tableFinish = false;
            }
        }
    }

    // znp.version >= 3.30:
    for (int i = 0; i < sizeof(exIds) / sizeof(exIds[0]); ++i)
    {
        if (exIds[i] != ExNvIds::LEGACY) // Skip the LEGACY items, we did them above
        {
            for (uint16_t sub_id = 0; sub_id < 65536; ++sub_id)
            {
                String msg = "Nv [E] ID: " + String(exIds[i], HEX) + ", sub: " + String(sub_id, HEX);
                logFunction(msg);

                if (!_nvram_ex_delete(exIds[i], sub_id))
                {
                    break;
                }
            }
        }
    }

    logFunction("NVRAM erase finish. Restarting...");

    restart();

    return success;
}