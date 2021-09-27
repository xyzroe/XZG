#include <Arduino.h>
#include <ArduinoJson.h>
//#include <ETH.h>
//#include "WiFi.h"
//#include <WebServer.h>
//#include "FS.h"
//#include "LITTLEFS.h"
#include "web.h"
#include "config.h"
#include "log.h"
#include "etc.h"
#include "zigbee.h"
//#include <Update.h>
//#include "html.h"

void startDialogZig()
{
}

void sendByteZig(uint8_t bytes[10], int length)
{

    for (int i = 0; i < length; i++)
    {
        sendBytebyByteZig(bytes[i]);
    }
        Serial2.flush();
    //char output_sprintf[2];

    //uint8_t cmd[1];
    //cmd[0] = one_byte;
    //DEBUG_PRINTLN(cmd[0]);
    //Serial2.write(one_byte, length);
    /*
  String buff = "";

  printLogTime();

  logPush('-');
  logPush('>');

  sprintf(output_sprintf, "%02x", cmd[0]);
  logPush(' ');
  logPush(output_sprintf[0]);
  logPush(output_sprintf[1]);

  logPush('\n');
*/
    //serverWeb.send(200, F("text/html"), "");
}

void sendBytebyByteZig(uint8_t one_byte)
{
    //char output_sprintf[2];

    //uint8_t cmd[1];
    //cmd[0] = one_byte;
    //DEBUG_PRINTLN(cmd[0]);
    
    uint8_t cmd[1];
    cmd[0] = one_byte;
    Serial2.write(cmd,1);
    delay(50);

    /*
  String buff = "";

  printLogTime();

  logPush('-');
  logPush('>');

  sprintf(output_sprintf, "%02x", cmd[0]);
  logPush(' ');
  logPush(output_sprintf[0]);
  logPush(output_sprintf[1]);

  logPush('\n');
*/
    //serverWeb.send(200, F("text/html"), "");
}

void cmdGetZigVersion()
{
    uint8_t cmd[10];
    cmd[0] = 0x55;
    cmd[1] = 0x55;
    sendByteZig(cmd, 2);
    /*
  //Serial2.flush();
  for (int i = 0; i < 2; i++)
  {
    sendByteZig(cmd[i]);
  }
  //delay(250);
*/
    //delay(250);
    uint8_t cmd2[10];
    cmd2[0] = 0x03;
    cmd2[1] = 0x28;
    cmd2[2] = 0x28;
    //sendByteZig(cmd2, 3);
    //Serial2.flush();
    /*
  for (int i = 0; i < 3; i++)
  {
    sendByteZig(cmd2[i]);
  }
  */
}

void cmdZigRestart()
{

    uint8_t cmd[10];
    cmd[0] = 0x55;
    cmd[1] = 0x55;

    sendByteZig(cmd, 2);
    /*
  //Serial2.flush();
  for (int i = 0; i < 2; i++)
  {
    sendByteZig(cmd[i]);
  }
  //delay(250);
*/
    delay(250);
    uint8_t cmd2[10];
    cmd2[0] = 0x03;
    cmd2[1] = 0x25;
    cmd2[2] = 0x25;
    sendByteZig(cmd2, 3);

    /*
  uint8_t cmd[2];
  cmd[0] = 0x55;
  cmd[1] = 0x55;

  //Serial2.flush();
  for (int i = 0; i < 2; i++)
  {
    sendByteZig(cmd[i]);
  }
  delay(250);

  uint8_t cmd2[3];
  cmd2[0] = 0x03;
  cmd2[1] = 0x25;
  cmd2[2] = 0x25;

  //Serial2.flush();
  for (int i = 0; i < 3; i++)
  {
    sendByteZig(cmd2[i]);
  }
  */
}
