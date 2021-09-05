#include <Arduino.h>
#include <etc.h>

void getReadableTime(String &readableTime)
{
    unsigned long currentMillis;
    unsigned long seconds;
    unsigned long minutes;
    unsigned long hours;
    unsigned long days;

    currentMillis = millis();
    seconds = currentMillis / 1000;
    minutes = seconds / 60;
    hours = minutes / 60;
    days = hours / 24;
    currentMillis %= 1000;
    seconds %= 60;
    minutes %= 60;
    hours %= 24;

    readableTime = String(days) + " d ";
    
    if (hours < 10)
    {
        readableTime += "0";
    }
    readableTime += String(hours) + ":";

    if (minutes < 10)
    {
        readableTime += "0";
    }
    readableTime += String(minutes) + ":";

    if (seconds < 10)
    {
        readableTime += "0";
    }
    readableTime += String(seconds) + "";
}