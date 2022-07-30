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
#include <cstdint>
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
        if (! this->reset())
            return false;

        this->m_startTime = millis();
        this->m_delay = LTR_329ALS_PARAMS::getInitialDelayMs();

        this->setState(State::PowerOn);

        // TODO(tmm@mcci.com): we should use an explicit FSM so that
        // we can embed this in a pollable object and NOT waste battery
        // while polling.
        while ((std::uint32_t)millis() - this->m_startTime < this->m_delay)
            ;

        this->setState(State::Initial);

        // for power reasons, we do NOT set "active" mode. We leave
        // the sensor in sleep mode until it's time to make a measurement.

        // set gain, measurement and integration time for white LED
        // This only sets register images; it doesn't write to the sensor.
        if (! this->configure(
                this->kInitialGain,
                this->kInitialMeasurementRate,
                this->kInitialIntegrationTime
                ))
            {
            // last error was set. Set state to uniitialized.
            this->setState(State::Uninitialized);
            return false;
            }

        this->m_startTime = millis();
        this->m_delay = LTR_329ALS_PARAMS::getWakeupDelayMs();

        // TODO(tmm@mcci.com): we should use an explicit FSM so that
        // we can embed this in a pollable object and NOT waste battery
        // while polling.
        while ((std::uint32_t)millis() - this->m_startTime < this->m_delay)
            /* don't put this semicolon on previous line! */;

        this->setState(State::Idle);
        }

    return result;
    }

#undef FUNCTION

void Ltr_329als::end(void)
    {
    if (this->isRunning())
        this->setState(State::Uninitialized);

    if (this->setStandby())
        this->setState(State::End);
    }

bool Ltr_329als::reset()
    {
    this->setState(State::Uninitialized);

    return this->writeRegister(
                Register_t::ALS_CONTR,
                AlsContr_t(0).setReset(true).getValue()
                );
    }

bool Ltr_329als::setStandby()
    {
    auto fResult = this->writeRegister(Register_t::ALS_CONTR, this->m_control.setActive(false).getValue());

    this->setState(fResult ? State::Idle : State::Uninitialized);
    return fResult;
    }

bool
Ltr_329als::readProductInfo(
    void
    )
    {
    std::uint8_t uPartId;
    std::uint8_t uManufacId;

    if (! this->readRegister(Register_t::PART_ID, uPartId))
        return false;

    if (! this->readRegister(Register_t::MANUFAC_ID, uManufacId))
        return false;

    this->m_partid = PartID_t(uPartId);
    this->m_manufacid = ManufacID_t(uManufacId);

    if (this->m_partid.getPartID() != m_partid.kPartID ||
        this->m_manufacid.getManufacID() != m_manufacid.kManufacID)
        {
        return this->setLastError(Ltr_329als::Error::PartIdMismatch);
        }

    return true;
    }

bool
Ltr_329als::configure(
    AlsGain_t::Gain_t g,
    AlsMeasRate_t::Rate_t r,
    AlsMeasRate_t::Integration_t iTime
    )
    {
    if (! (AlsGain_t::isGainValid(g) && AlsMeasRate_t::isRateValid(r) && AlsMeasRate_t::isIntegrationValid(iTime)))
        return this->setLastError(Error::InvalidParameter);

    // repeat can't be less than intergation time; if it is, integration time is reduced.
    if (r < iTime)
        return this->setLastError(Error::InvalidParameter);

    auto const state = this->getState();
    if (state == State::Single || state == State::Continuous)
        return this->setLastError(Error::Busy);

    this->m_control = this->m_control.setGain(g);

    this->m_measrate =
        AlsMeasRate_t(0)
            .setRate(this->m_userRate)
            .setIntegration(this->m_userIntegration)
            ;

    return true;
    }

bool
Ltr_329als::startMeasurement(bool fSingle)
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
        auto measrate = this->m_measrate;

        if (fSingle)
            // set the repeat rate really low.
            measrate = measrate.setRate(2000);

        this->m_control = this->m_control
                                .setActive(true)
                                .setReset(false)
                                ;

        if (! this->writeRegister(Register_t::ALS_MEAS_RATE, measrate.getValue()))
            return false;

        if (this->writeRegister(Register_t::ALS_CONTR, this->m_control.getValue()))
            {
            // we started.
            this->m_startTime = millis();
            this->m_pollTime = this->m_startTime;
            this->m_rawChannels.init();
            this->m_rawChannels.setMeasRate(measrate);
            this->setState(fSingle ? State::Single : State::Continuous);
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

    if (this->getState() == State::Single ||
        this->getState() == State::Continuous)
        {
        auto const now = millis();

        // is it time to start talking to the device?
        if (now - this->m_startTime < this->m_rawChannels.getIntegrationTime())
            {
            // not yet
            this->m_pollTime = now - 10;
            fError = false;
            return this->setLastError(Error::Busy);
            }

        // check the Als data status
        if (now - this->m_pollTime < 10)
            {
            fError = false;
            return this->setLastError(Error::Busy);
            }

        if (! this->readDataStatus())
            {
            // data error occurred.
            this->setState(State::Uninitialized);
            return false;
            }

        // don't poll for another 10 ms.
        this->m_pollTime = now;

        if (! (this->m_status.getNew() && this->m_status.getValid()))
            {
            // check for timeout.
            if (now - this->m_startTime > 2 * this->m_rawChannels.getIntegrationTime())
                {
                fError = true;
                this->setState(State::Uninitialized);
                return this->setLastError(Error::TimedOut);
                }
            else
                {
                fError = false;
                return this->setLastError(Error::Busy);
                }
            }

        if (! this->readRegisters(
                        Register_t::ALS_DATA_CH1_0,
                        this->m_rawChannels.getDataPointer(),
                        this->m_rawChannels.getDataSize()
                        ))
            {
            // last error is set
            this->setState(State::Uninitialized);
            return false;
            }

        // record the status
        this->m_rawChannels.setStatus(this->m_status);

        // change state.
        if (this->getState() == State::Single)
            {
            // idle the device; changes state back to idle.
            return this->setStandby();
            }
        else
            {
            // continuous mode keeps measuring. Set up a timeout.
            this->m_startTime = now;
            this->m_pollTime = now;
            return true;
            }
        }
    else
        {
        fError = true;
        return this->setLastError(Error::NotMeasuring);
        }
    }

float Ltr_329als::getLux()
    {
    bool fError;
    float ambientLight;

    // computeLux does everything except set last error...
    ambientLight = this->m_rawChannels.computeLux(fError);

    // so we do that here.
    if (fError)
        {
        this->setLastError(Error::InvalidData);
        }

    // computeLux sets ambient light to zero in case of error.
    return ambientLight;
    }

// protected
bool Ltr_329als::readDataStatus()
    {
    std::uint8_t uStatus;

    if (! this->readRegister(Register_t::ALS_STATUS, uStatus))
        return false;

    this->m_status = AlsStatus_t(uStatus);
    return true;
    }

// protected
bool Ltr_329als::readRegister(Register_t r, std::uint8_t &v)
    {
    return readRegisters(r, &v, 1);
    }

// protected
bool Ltr_329als::readRegisters(Register_t r, std::uint8_t *pBuffer, size_t nBuffer)
    {
    if (pBuffer == nullptr || nBuffer > 32)
        return this->setLastError(Error::InternalInvalidParameter);

    this->m_wire->beginTransmission(LTR_329ALS_PARAMS::Address);
    if (this->m_wire->write((uint8_t)r) != 1)
        {
        return this->setLastError(Error::I2cReadRequest);
        }
    if (this->m_wire->endTransmission() != 0)
        {
        return this->setLastError(Error::I2cReadRequest);
        }

    auto nReadFrom = this->m_wire->requestFrom(LTR_329ALS_PARAMS::Address, std::uint8_t(nBuffer));

    if (nReadFrom != nBuffer)
        return this->setLastError(Error::I2cReadRequest);
    auto nResult = unsigned(this->m_wire->available());

    if (nResult > nBuffer)
        return this->setLastError(Error::I2cReadLong);

    for (unsigned i = 0; i < nResult; ++i)
        pBuffer[i] = this->m_wire->read();

    if (nResult != nBuffer)
        return this->setLastError(Error::I2cReadShort);

    return true;
    }

// protected
bool Ltr_329als::writeRegister(Register_t r, std::uint8_t v)
    {
    this->m_wire->beginTransmission(LTR_329ALS_PARAMS::Address);

    if (this->m_wire->write((uint8_t)r) != 1)
        return this->setLastError(Error::I2cWriteBufferFailed);
    if (this->m_wire->write((uint8_t)v) != 1)
        return this->setLastError(Error::I2cWriteBufferFailed);

    if (this->m_wire->endTransmission() != 0)
        return this->setLastError(Error::I2cWriteFailed);

    return true;
    }

/****************************************************************************\
|   String handling for error routines
\****************************************************************************/

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
