/*

Module: ltr_329als_simple.ino

Function:
        Simple example for LTR light sensors.

Copyright and License:
        See accompanying LICENSE file.

Author:
        Dhinesh Kumar Pitchai, MCCI Corporation   July 2022

*/

#include <mcci_ltr_329als.h>

#include <Arduino.h>
#include <Wire.h>
#include <cstdint>

/****************************************************************************\
|
|   Manifest constants & typedefs.
|
\****************************************************************************/

using namespace Mcci_Ltr_329als;

/****************************************************************************\
|
|   Read-only data.
|
\****************************************************************************/

/****************************************************************************\
|
|   Variables.
|
\****************************************************************************/

bool fLed;
Ltr_329als gLtr {Wire};

/****************************************************************************\
|
|   Code.
|
\****************************************************************************/

void printFailure(const char *pMessage)
    {
    for (;;)
        {
        Serial.print(pMessage);
        Serial.print(", error: ");
        Serial.print(gLtr.getLastErrorName());
        Serial.print("(");
        Serial.print(std::uint8_t(gLtr.getLastError()));
        Serial.print(")");
        delay(2000);
        }
    }

// format a uint64_t into a buffer and return pointer to first byte.
char *formatUint64(char *pBuf, size_t nBuf, std::uint64_t v, unsigned base)
    {
    char *p;

    if (pBuf == nullptr || nBuf == 0)
        return nullptr;

    p = pBuf + nBuf;
    *--p = '\0';
    while (p > pBuf)
        {
        unsigned digit;
        
        digit = unsigned(v % base);
        v = v / base;
        --p;

        if (digit < 10)
            *p = '0' + digit;
        else
            *p = 'A' + digit - 10;

        if (v == 0)
            break;
        }

    if (p == pBuf && v != 0)
        *p = '*';

    return p;
    }

void setup()
    {
    Serial.begin(115200);

    // wait for USB to be attached.
    while (! Serial)
        yield();

    /* bring up the LED */
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 1);
    fLed = true;

    Serial.println("LTR329-ALS01 Simple Test");
    // let message get out.
    delay(1000);

    if (! gLtr.begin())
        {
        printFailure("gLtr.begin() failed");
        }
    }

void loop()
    {
    bool fError;
    float currentLux;

    // toggle the LED
    fLed = !fLed;
    digitalWrite(LED_BUILTIN, fLed);

    // start a measurement
    if (! gLtr.startSingleMeasurement())
        printFailure("gLtr.startSingleMeasurement() failed");

    // wait for measurement to complete.
    while (! gLtr.queryReady(fError))
        {
        if (fError)
            printFailure("queryReady() failed");
        }

    // get the results.
    currentLux = gLtr.getLux();

    // put the sensor to sleep.
    gLtr.end();

    // display the results by fetching then displaying.
    Serial.print("light=");
    Serial.print(currentLux);
    Serial.println(" lux");

    delay(2000);
    }
