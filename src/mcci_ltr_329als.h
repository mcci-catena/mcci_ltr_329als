/*

Module: mcci_ltr_329als.h

Function:
    Top-level include file for MCCI LTR-329ALS library.

Copyright and License:
    See accompanying LICENSE file for copyright and license information.

Author:
    Terry Moore, MCCI Corporation   July 2022

*/

/// \file

#ifndef _mcci_ltr_329als_h_
#define _mcci_ltr_329als_h_ /* prevent multiple includes */

#include <stdint.h>
#pragma once

#include <cstdint>
#include <Wire.h>
#include "mcci_ltr_329als_regs.h"

/// \brief namespace for this library
namespace Mcci_Ltr_329als {

using namespace Mcci_Ltr_329als_Regs;

// in order to be independent of the big MCCI Catena libarry, we define
// a number of things again here.

///
/// \brief Represent a Semantic Version constant numerically
///
/// \details
///     Objects of type \c Version_t represent a subset of Semantic Version values,
///     as defined by the Semantic Version 2.0 specification. \c major, \c minor, and
///     \c patch may range from 0 to 255, and have the same meaning as in the specification.
///     \c prerelease, if not zero, indicates that this version is a pre-release for
///     the specified \c major.minor.patch release. Relational operators are defined
///     so that pre-releases will compare less than the corresponding releases.
///
class Version_t
    {
public:
    /// \brief create a version constant uint32_t
    static constexpr uint32_t makeVersion(uint8_t major, uint8_t minor, uint8_t patch, uint8_t prerelease = 0)
        {
        return (uint32_t(major) << 24) | (uint32_t(minor) << 16) | (uint32_t(patch << 8)) | prerelease;
        }

    /// \brief the size of a text version (without leading 'v', with trailing null)
    static constexpr size_t kVersionBufferSize = sizeof("255.255.255-pre255");

    /// \brief construct a \c Version_t object from parts
    constexpr Version_t(uint8_t major, uint8_t minor, uint8_t patch, uint8_t prerelease = 0)
        : m_version(makeVersion(major, minor, patch, prerelease)) {}

    /// \brief default constuctor
    Version_t() {};

    /// \brief construct a \c Version_t object given a code.
    constexpr Version_t(uint32_t versionCode)
        : m_version(versionCode) {}

    /// \brief return the version constant as a uint32_t. Can't be compared!
    constexpr uint32_t getUint32() const
        {
        return this->m_version;
        }

    /// \brief return the version as a sequential constant. Can be compared, but doesn't match what was given.
    constexpr uint32_t getOrdinal() const
        {
        return (this->m_version & 0xFFFFFF00u) | ((this->m_version - 1) & 0xFFu);
        }

    /// \brief basic relational comparison operator for \c Version_t objects
    friend constexpr bool operator<(const Version_t &lhs, const Version_t &rhs) { return lhs.getOrdinal() < rhs.getOrdinal(); }

    /// \brief basic identity comparison operator for \c Version_t objects
    friend constexpr bool operator==(const Version_t &lhs, const Version_t &rhs) { return lhs.getUint32() == rhs.getUint32(); }

    /// \brief return the Semantic Version major version of a \c Version_t value
    constexpr uint8_t getMajor() const
        {
        return uint8_t(this->m_version >> 24);
        }

    /// \brief return the Semantic Version minor version of a \c Version_t value
    constexpr uint8_t getMinor() const
        {
        return uint8_t(this->m_version >> 16);
        }

    /// \brief return the Semantic Version patch number from a \c Version_t value.
    constexpr uint8_t getPatch() const
        {
        return uint8_t(this->m_version >> 8);
        }

    /// \brief return the Semantic Version pre-release from a \c Version_t value.
    constexpr uint8_t getPrerelease() const
        {
        return uint8_t(this->m_version >> 0);
        }

    /// \brief test whether a version is a pre-release
    constexpr bool isPrerelease() const
        {
        return this->getPrerelease() != 0;
        }

    /// \brief build a version string in a buffer
    size_t toBuffer(char *pBuffer, size_t nBuffer) const;

private:
    uint32_t m_version;     ///< the encoded version number
    };

/// derived relational operator
constexpr bool operator> (const Version_t& lhs, const Version_t& rhs){ return rhs < lhs; }
/// derived relational operator
constexpr bool operator<=(const Version_t& lhs, const Version_t& rhs){ return !(lhs > rhs); }
/// derived relational operator
constexpr bool operator>=(const Version_t& lhs, const Version_t& rhs){ return !(lhs < rhs); }
/// derived identity operator
constexpr bool operator!=(const Version_t& lhs, const Version_t& rhs){ return !(lhs == rhs); }


/// \brief instance object for LTR-329als
class Ltr_329als
    {
public:
    /// \brief the version number for this version of the library.
    static constexpr Version_t kVersion = Version_t(1, 0, 0, 4);

    ///
    /// \brief initial sensor gain.
    ///
    /// The initial sensor gain is 1. This is chosen to allow measurement of full scale range.
    ///
    static constexpr AlsGain_t::Gain_t kInitialGain = 1;

    ///
    /// \brief initial integration time.
    ///
    /// The initial integration time is 100 ms. This value is chosen to allow measurement
    /// of the full scale range of the sensor.
    ///
    static constexpr AlsMeasRate_t::Integration_t kInitialIntegrationTime = 100;

    ///
    /// \brief initial measurment rate in ms per measurement.
    ///
    /// The inital measurement rate is 1000 ms. This value is chosen so we have
    /// time to put the device back to sleep after single measurements, which is
    /// our intended mode of operation.
    ///
    static constexpr AlsMeasRate_t::Rate_t kInitialMeasurementRate = 1000;


    ///
    /// \brief Error codes
    ///
    enum class Error : std::uint8_t
        {
        Success = 0,                ///< no error
        InvalidParameter,           ///< invalid parameter to API
        Busy,                       ///< busy doing a measurement
        NotMeasuring,               ///< not measuring; will never become ready.
        TimedOut,                   ///< measurement timed out
        InvalidData,                ///< Lux data couldn't be converted.
        PartIdMismatch,             ///< part ID did not match library
        I2cReadRequest,             ///< read request failed to start.
        I2cReadShort,               ///< too few bytes from read
        I2cReadLong,                ///< too many bytes from read
        I2cWriteFailed,             ///< I2C write failure
        I2cWriteBufferFailed,       ///< I2C write buffer fill failure
        NoWire,                     ///< internal error: the wire pointer is null.
        InternalInvalidParameter,   ///< internal error: invalid parmaeter
        Uninitialized,              ///< internal error: driver is not running
        };

private:
    /// \brief table of error messages
    ///
    /// \internal
    /// this is internal -- centralize it but require that clients call the
    /// public method (which centralizes the strings and the search)
    static constexpr const char * const m_szErrorMessages =
        "Success"                   "\0"
        "InvalidParameter"          "\0"
        "Busy"                      "\0"
        "NotMeasuring"              "\0"
        "TimedOut"                  "\0"
        "InvalidData"               "\0"
        "PartIdMismatch"            "\0"
        "I2cReadRequest"            "\0"
        "I2cReadShort"              "\0"
        "I2cReadLong"               "\0"
        "I2cWriteFailed"            "\0"
        "I2cWriteBufferFailed"      "\0"
        "NoWire"                    "\0"
        "InternalInvalidParameter"  "\0"
        "Uninitialized"             "\0"
        ;

public:
    /// \brief state of the measurement engine
    enum class State : std::uint8_t
        {
        Uninitialized,      ///< this->begin() has never succeeded.
        End,                ///< this->begin() succeeded, followed by this->end()
        PowerOff,           ///< this->begin() succeeded, power is off.
        PowerOn,            ///< power on, delaying 100 ms.
        Initial,            ///< initial after begin (standby mode)
        Idle,               ///< idle (not measuring, active mode)
        Single,             ///< running a single measurement.
        Continuous,          ///< continuous measurement running, no data available.
        Ready,              ///< continuous measurement running, data availble.
        };

private:
    /// \brief table of state names, '\0'-separated.
    ///
    /// \internal
    /// this is internal -- centralize it but require that clients call the
    /// public method (which centralizes the strings and the search)
    static constexpr const char * const m_szStateNames =
        "Uninitialized" "\0"
        "End"           "\0"
        "PowerOff"      "\0"
        "PowerOn"       "\0"
        "Initial"       "\0"
        "Idle"          "\0"
        "Single"        "\0"
        "Continuous"     "\0"
        "Ready"         "\0"
        ;

public:
    ///
    /// \brief the constructor
    ///
    /// \param [in] myWire is the TwoWire bus to use for this sensor.
    ///
    Ltr_329als(TwoWire &myWire)
        : m_wire(&myWire)
        , m_lastError(Error::Success)
        {}

    // uses default destructor

    // neither copyable nor movable
    Ltr_329als(const Ltr_329als&) = delete;
    Ltr_329als& operator=(const Ltr_329als&) = delete;
    Ltr_329als(const Ltr_329als&&) = delete;
    Ltr_329als& operator=(const Ltr_329als&&) = delete;

    ///
    /// \brief Power up the light sensor and start operation.
    ///
    /// \return
    ///     \c true for success, \c false for failure (in which case the the last
    ///     error is set to the error reason).
    ///
    bool begin();

    /// \brief read product information
    bool readProductInfo(void);

    /// \brief configure measurement
    bool configure(AlsGain_t::Gain_t g, AlsMeasRate_t::Rate_t r, AlsMeasRate_t::Integration_t iTime);

    /// \brief abstract type: holds a count of milliseconds
    using ms_t = std::uint32_t;

    /// \brief abstract type: holds register address
    using Register_t = LTR_329ALS_PARAMS::Reg_t;

    /// \brief start a single measurement.
    bool startSingleMeasurement()
        {
        return startMeasurement(true);
        }

    /// \brief start a single measurement.
    bool startMeasurement(bool fSingle = true);

    ///
    /// \brief find out whether a measurement is ready
    ///
    /// \param [out] fError used to distinguish
    ///         hard errors from "not ready"
    ///
    /// \return
    ///     \c true if a measurement is ready and in the
    ///     buffer. \c false if a measurement is not
    ///     yet ready. See details.
    ///
    /// \details
    ///     In the normal course of events, you'll start
    ///     a measurement using startMeasurement(), then
    ///     poll queryReady() until the value becomes
    ///     \c true.  However, if a hard error occurs,
    ///     queryReady() will never return \c true. To
    ///     write robust code, you must check fError
    ///     to distinguish between a hard error and
    ///     not-yet error.
    ///
    ///     When queryReady() returns \c true, you
    ///     may either call getLux() to conver the
    ///     data to lux, or else save \refitem m_data
    ///     in a local variable, and convert it
    ///     later.
    ///
    bool queryReady(bool &fError);

    ///
    /// \brief Convert the data in the buffer to lux, and return.
    ///
    /// If the data is not valid, 0.0f is returned, and the last
    /// error is set. If the data is valid, the computed lux are
    /// returned, and the last error is left unchanged.
    ///
    /// The conversion takes into account the selected gain and
    /// integration time.
    ///
    /// \return light value in lux.
    ///
    float getLux();

    /// \brief reset and stop any ongoing measurement
    bool reset();

    /// \brief end operation;
    void end();

    /// \brief convert a state value to a state name.
    static const char *getStateName(State s);

    /// \brief return name of current state.
    const char *getCurrentStateName() const
        {
        return getStateName(this->getState());
        }

    /// \brief return true if the driver is running.
    bool isRunning() const
        {
        return this->m_state > State::End;
        }

    /// \brief return current state of driver.
    State getState() const { return this->m_state; }

    /// \brief get the last error reported from this instance
    Error getLastError() const
        {
        return this->m_lastError;
        }

    /// \brief set the last error code.
    bool setLastError(Error e)
        {
        this->m_lastError = e;
        return e == Error::Success;
        }

    /// \brief return a string for a given error code.
    static const char *getErrorName(Error e);

    /// \brief return the name of the last error.
    const char *getLastErrorName() const
        {
        return getErrorName(this->m_lastError);
        }

    /// \brief return a const reference to the data regs
    const DataRegs_t &getRawData() const
        {
        return this->m_rawChannels;
        }

protected:
    /// \brief put the LTR-329ALS into low-power standby
    bool    setStandby();

    ///
    /// \brief Change state of driver.
    ///
    /// \param [in] s is the new state
    ///
    /// \details
    ///     This function changes the recorded state of the driver instance.
    ///     When debugging, this might also log state changes; you can do that
    ///     by overriding this method in a derived class.
    ///
    virtual void setState(State s)
        {
        this->m_state = s;
        }

    ///
    /// \brief Make sure the driver is running
    ///
    /// If not running, set last error to Error::Uninitialized, and return \c false.
    /// Otherwise return \c true.
    ///
    /// Normally used in the following pattern:
    ///
    /// \code
    ///     if (! this->checkRunning())
    ///         return false;
    ///     // otherwise do some work...
    /// \endcode
    ///
    bool checkRunning()
        {
        if (! this->isRunning())
            return this->setLastError(Error::Uninitialized);
        else
            return true;
        }

    ///
    /// \brief Write a byte to a given register.
    ///
    /// \param [in] r selects the register to write
    /// \param [in] v is the value to be written.
    ///
    /// \return
    ///     \c true for success, \c false for failure. The
    ///     last error is set in case of error.
    ///
    bool writeRegister(Register_t r, std::uint8_t v);

    ///
    /// \brief read a byte from a given register.
    ///
    /// \param [in] r indicates the register to be read.
    /// \param [out] v is set to the value read (if no error).
    ///
    /// \return
    ///     \c true for success, \c false for failure. The
    ///     last error is set in case of error.
    ///
    bool readRegister(Register_t r, std::uint8_t &v);

    ///
    /// \brief read a series of bytes starting with a given register.
    ///
    /// \param [in] r indicates the starting register to be read.
    /// \param [out] pBuffer points to the buffer to receive the data
    /// \param [in] nBuffer is the number of bytes to read.
    ///
    /// \return
    ///     \c true for success, \c false for failure. The
    ///     last error is set in case of error.
    ///
    bool readRegisters(Register_t r, std::uint8_t *pBuffer, size_t nBuffer);


    ///
    /// \brief Set up the control and measurement registers
    ///
    /// This routine should be called before kicking off a measurement.
    /// It ensures that the sensors control and measrate registers
    /// match what the user requested.
    ///
    bool setupMeasurement();

    ///
    /// \brief update m_status from the status register
    ///
    /// \details
    ///     The ALS_STATUS register is read and stored into \refitem m_status.
    ///
    /// \return
    ///     \c true for success, \c false for failure. The
    ///     last error is set in case of error.
    ///
    bool readDataStatus();

    //
    // The local variables
    //
private:
    TwoWire     *m_wire;                ///< pointer to I2C bus
    AlsGain_t::Gain_t m_userGain;       ///< user-requested gain
    AlsMeasRate_t::Integration_t m_userIntegration;     ///< user-reqeusted integration period
    AlsMeasRate_t::Rate_t m_userRate;   ///< user-reqeusted measurement repeat rate
    ms_t        m_startTime;            ///< when the last measurement was started
    ms_t        m_pollTime;             ///< last time mesurement was polled
    ms_t        m_delay;                ///< ms to delay
    Error       m_lastError;            ///< last error
    State       m_state;                ///< state of measurement engine
    AlsContr_t  m_control;              ///< control register
    AlsMeasRate_t m_measrate;               ///< rate/integration register
    AlsStatus_t m_status;               ///< status register
    DataRegs_t  m_rawChannels;          ///< last raw data result.
    AlsStatus_t  m_saveStatus;          ///< status from last measurement
    AlsMeasRate_t m_saveMeasRate;       ///< AlsMeasRate_t from last measurement
    PartID_t    m_partid;               ///< part id register
    ManufacID_t m_manufacid;            ///< manufacturer id register
    };

} // end namespace Mcci_Ltr_329als

#endif /* _mcci_ltr_329als_h_ */
