/*

Module: mcci_ltr_329als.cpp

Function:
    Implementation code for LTR-329ALS light sensor library.

Copyright and License:
    See accompanying LICENSE file for copyright and license information.

Author:
    Terry Moore, MCCI Corporation   July 2022

*/

#include "mcci_ltr_329als.h"
#include <Arduino.h>
#include <stdint.h>

using namespace Mcci_Ltr_329als;

/****************************************************************************\
|
|   Manifest constants & typedefs.
|
\****************************************************************************/

static const char *scanMultiSzString(const char *p, unsigned eIndex);

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


/*

Name:	Ltr_329als::begin()

Function:
    Start operating the LTR-329ALS

Definition:
    bool Ltr_329als::begin(
        void
        );

Description:
    If the driver is already running, this function succeed.
    Otherwise, this function assumes that the sensor has just
    been powered up, and records the time.
    Probe and so forth are delayed until the 100ms delay has elapsed.

Returns:
    true for success, false for failure. If any errors, then
    Ltr_329als::getLastError() will return the error cause.

Notes:


*/

#define FUNCTION "Ltr_329als::begin"

bool
Ltr_329als::begin(
    void
    )
    {
    // if no Wire is bound, fail.
    if (this->m_wire == nullptr)
        return this->setLastError(Error::NoWire);

    if (this->isRunning())
        return true;

    this->m_wire->begin();
    bool result = this->readProductInfo();

    if (result && (this->getState() == State::Uninitialized))
        {
        this->m_startTime = millis();
        this->m_delay = LTR_329ALS_PARAMS::getInitialDelayMs();

        this->setState(State::Initial);
        }

    return result;
    }

#undef FUNCTION

bool
Ltr_329als::readProductInfo(
    void
    )
    {
    uint8_t PartId;
    uint8_t ManufacId;

    PartId = this->readRegister(Register_t::PART_ID);
    ManufacId = this->readRegister(Register_t::MANUFAC_ID);

    if ((PartId >> 4) != m_partid.kPartID ||
        ManufacId != m_manufacid.kManufacID)
        return false;

    return true;
    }

bool
Ltr_329als::configure(
    AlsGain_t::Gain_t g,
    AlsMeasRate_t::Rate_t r,
    AlsMeasRate_t::Integration_t iTime)
    {
    this->m_userGain = g;
    AlsContr_t(0).setGain(this->m_userGain);

    this->m_userRate = r;
    this->m_userIntegration = iTime;
    AlsMeasRate_t(0)
            .setRate(this->m_userRate)
            .setIntegration(this->m_userIntegration)
            ;

    if (! this->checkRunning())
        return false;

    if (! this->writeRegister(Register_t::ALS_MEAS_RATE, this->m_rate.getValue()))
        return false;

    if (this->writeRegister(Register_t::ALS_CONTR, this->m_control.getValue()))
        {
        // we started.
        this->m_startTime = millis();
        this->m_saveMeasRate = this->m_rate;
        this->m_saveStatus = AlsStatus_t(0);
        this->setState(State::Single);
        return true;
        }
    }

bool
Ltr_329als::readMeasurement()
    {
    if (! this->checkRunning())
        return false;

    if (this->getState() != State::Idle)
        {
        // busy
        return this->setLastError(Error::Busy);
        }
    else
        {
        this->m_rawChannels.m_data[1] = this->readRegister(Register_t::ALS_DATA_CH1_0);
        this->m_rawChannels.m_data[0] = this->readRegister(Register_t::ALS_DATA_CH1_1);
        this->m_rawChannels.m_data[3] = this->readRegister(Register_t::ALS_DATA_CH0_0);
        this->m_rawChannels.m_data[2] = this->readRegister(Register_t::ALS_DATA_CH0_1);
        }
    }

bool
Ltr_329als::startSingleMeasurement()
    {
    if (! this->checkRunning())
        return false;

    if (this->getState() == State::Single)
        // already measuring
        return true;
    else if (this->getState() != State::Idle)
        {
        // busy
        return this->setLastError(Error::Busy);
        }
    else
        {
        // set the state of the device and trigger a measurement
        this->m_rate = AlsMeasRate_t(0)
                                .setRate(2000)
                                .setIntegration(this->m_userIntegration)
                                ;
        this->m_control = AlsContr_t(0)
                                .setGain(this->m_userGain)
                                .setActive(true)
                                .setReset(false)
                                ;

        if (! this->writeRegister(Register_t::ALS_MEAS_RATE, this->m_rate.getValue()))
            return false;

        if (this->writeRegister(Register_t::ALS_CONTR, this->m_control.getValue()))
            {
            // we started.
            this->m_startTime = millis();
            this->m_saveMeasRate = this->m_rate;
            this->m_saveStatus = AlsStatus_t(0);
            this->setState(State::Single);
            return true;
            }
        }
    }

float Ltr_329als::getLux()
    {
    bool fError;
    float ambientLight;

    ambientLight = this->m_rawChannels.computeLux(&fError);
    }

bool Ltr_329als::queryReady(bool &fError)
    {
    if (! checkRunning())
        {
        fError = true;
        return false;
        }

    if (this->getState() == State::Ready)
        {
        fError = false;
        return true;
        }

    if (this->getState() == State::Idle)
        {
        fError = true;
        return this->setLastError(Error::NotMeasuring);
        }

    if ((std::int32_t)(millis() - this->m_startTime < this->m_currentIntegration)
        {
        fError = false;
        return this->setLastError(Error::Busy);
        }

    if (this->getState() == State::Single ||
        this->getState() == State::Triggered)
            {

            }
    }

static const char *scanMultiSzString(const char *p, unsigned eIndex)
    {
    // iterate based on error index.
    for (; eIndex > 0; --eIndex)
        {
        // stop when we get to empty string
        if (*p == '\0')
            break;
        // otherwise, skip this one.
        p += strlen(p) + 1;
        }

    // if we have a valid string, return it
    if (*p != '\0')
        return p;
    // otherwise indicate that the input wasn't valid.
    else
        return "<<unknown>>";
    }

const char * Ltr_329als::getErrorName(Ltr_329als::Error e)
    {
    return scanMultiSzString(m_szErrorMessages, unsigned(e));
    }

const char * Ltr_329als::getStateName(Ltr_329als::State s)
    {
    return scanMultiSzString(m_szStateNames, unsigned(s));
    }

/**** end of mcci_ltr_329als.cpp ****/
