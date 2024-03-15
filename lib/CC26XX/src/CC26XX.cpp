#include <Arduino.h>
#include "CC26XX.h"

CommandInterface::CommandInterface(Stream &serial) : _stream(serial) {}


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

// Функция для декодирования адреса из массива байтов
unsigned long CommandInterface::_decodeAddr(byte byte0, byte byte1, byte byte2, byte byte3)
{
    return ((unsigned long)byte3 << 24) | ((unsigned long)byte2 << 16) | ((unsigned long)byte1 << 8) | byte0;
}

// Функция для расчета контрольной суммы
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

CC26XX_detect::CC26XX_detect(Stream &serial) : CommandInterface(serial) {}

bool CC26XX_detect::begin(int CC_RST_PIN, int CC_BSL_PIN, int BSL_MODE)
{
    _CC_RST_PIN = CC_RST_PIN;
    _CC_BSL_PIN = CC_BSL_PIN;
    _BSL_MODE = BSL_MODE;

    pinMode(_CC_RST_PIN, OUTPUT);
    pinMode(_CC_BSL_PIN, OUTPUT);

    _enterBSLMode();
    if (!_sendSynch())
    {
        // Serial.print("begin NOT ok");
        return false;
    }

    
    return true;
}

void CC26XX_detect::_enterBSLMode()
{
    if (_BSL_MODE == 0)
    {
        digitalWrite(_CC_RST_PIN, LOW);
        digitalWrite(_CC_BSL_PIN, LOW);
        delay(250);
        digitalWrite(_CC_RST_PIN, HIGH);
        delay(2000);
        digitalWrite(_CC_BSL_PIN, HIGH);
        delay(1000);
        //Serial.println("enter bsl mode...");
    }

    // Другие режимы BSL_MODE могут быть добавлены здесь в будущем
}

String CC26XX_detect::detectChipInfo()
{
    uint32_t chip_id = _cmdGetChipId();

    // Serial.println(chip_id, HEX);
    //  Serial.println(chip_id_2, HEX);
    //  String chip_id_str = _getChipIdString(chip_id);

    // Serial.println(chip_id_str);

    // if (chip_id == 0xF000)
    //{
    // Serial.println("2652");
    // String chip_id_str_2 = _getChipIdString2652(chip_id_2);
    // Serial.println(chip_id_str_2);

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

    // Serial.println("Packege: " + _getPackage(user_id[2]));

    byte protocols = user_id[1] >> 4;
    // Serial.print("protocols: ");
    // Serial.println(protocols, HEX);

    String chip_str = "";
    if (protocols & PROTO_MASK_IEEE == PROTO_MASK_IEEE)
    {
        if (chip_id == 0x1202 && wafer_id == 0xBB77 && pg_rev == 0x1)
        {
            chip_str = "CC2652P7";
        }
        else if (chip_id == 0x3202 && wafer_id == 0xBB41 && pg_rev == 0x3)
        {
            chip_str = "CC2652P2";
        }
        else
        {
            chip_str = "Unknown ( " + String(chip_id, HEX) + " " + String(wafer_id, HEX) + " " + String(pg_rev, HEX) + " )";
        }
        // Serial.println(chip_str);
    }
    return chip_str;
}

