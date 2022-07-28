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
    if (this->getState() == State::Uninitialized)
        {
        this->m_startTime = millis();
        this->m_delay = LTR_329ALS_PARAMS::getInitialDelayMs();

        this->setState(State::Initializing);
        }
    }

#undef FUNCTION

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
                                .setActiveMode(true)
                                .setReset(false)
                                ;

        if (! this->writeRegister(Reg_t::ALS_MEAS_RATE, this->m_rate.getValue()))
            return false;

        if (this->writeRegister(Reg_t::ALS_CONTR, this->m_control.getValue()))
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
