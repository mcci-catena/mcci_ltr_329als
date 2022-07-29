/*

Module: mcci_ltr_329als_regs.h

Function:
    Register definitions for LTR-329ALS

Copyright and License:
    See accompanying LICENSE file for copyright and license information.

Author:
    Terry Moore, MCCI Corporation   July 2022

*/

#ifndef _mcci_ltr_329als_regs_h_
#define _mcci_ltr_329als_regs_h_    /* prevent multiple includes */

#include <cstdint>

#pragma once

/// \brief constants related to the LTR-329ALS registers
namespace Mcci_Ltr_329als_Regs {

    ///
    /// \brief An abstract class containing the basic constants
    ///     relevant to programming the LTR-329ALS sensor.
    ///
    /// \details
    ///     This class contains the I2C address, the register
    ///     offset, and the principal bit definitions for the
    ///     registers of the LTR-329ALS sensor. It is used
    ///     in turn by the specializations that model each
    ///     of the invidual registers.
    ///
    ///     It also encapsulates the basic template that is
    ///     used for handling bit manipulations of the registers
    ///     in a portable and reasonable type-safe way.
    ///
    class LTR_329ALS_PARAMS
        {
    public:
        /// \brief the I2C address of the LTR-329ALS.
        static constexpr std::uint8_t Address = 0x29;

        /// \brief register addresses within the LTR-329ALS.
        enum class Reg_t : std::uint8_t
            {
            ALS_CONTR       = 0x80,             ///< ALS operation mode register
            ALS_MEAS_RATE   = 0x85,             ///< ALS measurement rate control
            PART_ID         = 0x86,             ///< part number and revision ID
            MANUFAC_ID      = 0x87,             ///< manufacturer ID
            ALS_DATA_CH1_0  = 0x88,             ///< ALS measurement data channel 1, LSB
            ALS_DATA_CH1_1  = 0x89,             ///< ALS measurement data channel 1, MSB
            ALS_DATA_CH0_0  = 0x8A,             ///< ALS measurement data channel 0, LSB
            ALS_DATA_CH0_1  = 0x8B,             ///< ALS measurement data channel 0, MSB
            ALS_STATUS      = 0x8C,             ///< ALS new data status
            };

        /// \brief bits in the LTR-329ALS \c ALS_CONTR register
        enum class ALS_CONTR_BITS : std::uint8_t
            {
            MODE =  (1 << 0),                   ///< mode control (active / not-suspended)
            RESET = (1 << 1),                   ///< reset / not-reset
            GAIN = (7 << 2),                    ///< gain control
            };

        /// \brief bits in the LTR-329ALS \c ALS_MEAS_RATE register
        enum class ALS_MEAS_RATE_BITS : std::uint8_t
            {
            RATE = (3 << 0),                    ///< measurement rate
            TIME = (3 << 3),                    ///< integration time
            };

        /// \brief bits in the LTR-32lALS \c PART_ID register
        enum class PART_ID_BITS : std::uint8_t
            {
            REVID = 0xF << 0,                   ///< revision ID
            PARTNUM = 0xF << 4,                 ///< part number
            };

        /// \brief bits in the LTR-329ALS \c ALS_STATUS register
        enum class ALS_STATUS_BITS : std::uint8_t
            {
            NEW = 1 << 2,                       ///< new data if true
            GAIN = 7 << 4,                      ///< Data gain range
            INVALID = 1 << 7,                   ///< invalid data if true
            };

        ///
        /// \brief Return the required delay from reset to first operation,
        ///     in milliseconds.
        ///
        /// \note No allowance is made for variation over temperature and
        ///     voltage; the result is just the datasheet value.
        ///
        static constexpr std::uint32_t getInitialDelayMs()
            {
            return 100;
            }

        /// \brief return the required delay from standby to active,
        ///     in milliseconds.
        static constexpr std::uint32_t getWakeupDelayMs()
            {
            return 10;
            }

        static constexpr std::uint32_t getMaxInitialDelayMs()
            {
            // data sheet says 1000, but that's at 25c and 3.0V.  Allow some margin.
            return 1500;
            }

        ///
        /// \brief an abstract class to help with managing fields
        ///
        /// \details
        ///     All the register-image classes inherit from this class,
        ///     in order to get a common way to handle multi-bit field
        ///     within the register images.
        ///
        ///     We use this rather than use C++ bit fields, because
        ///     this is portable; bit order of C++ bit fields is not
        ///     portable.
        ///
        template <typename BASE_TYPE>
        class Field_t
            {
        private:
        protected:

            /// \brief extract the least-significant bit from a mask.
            template <typename T>
            static constexpr T fieldlsb(T fmask) { return ((fmask) & (~(fmask) + T(1))); }

            /// \brief given a mask and a value to place in that mask, shift value left appropriately.
            template <typename T>
            static constexpr T fieldvalue(T fmask, T val) { return (fieldlsb(fmask) * val) & fmask; }

            /// \brief given a mask and an image of the register, extract the bits from the field given
            /// by mask, and right-justify.
            template <typename T>
            static constexpr T fieldget(T fmask, T val) { return (val & fmask) / fieldlsb(fmask); }

            ///
            /// \brief given a mask, an image of the register, and a value, isert the bits from value
            /// into the register.
            ///
            /// \param [in] fmask specifies the bits to be changed
            /// \param [in] val is the input image of the register bits
            /// \param [in] fv is the right-justified value to be put into the bits given by fmask.
            ///
            /// \return the updated value.
            ///
            template <typename T, typename TV>
            static constexpr T fieldset(T fmask, T val, TV fv) { return (val & ~fmask) | (fv * fieldlsb(fmask)); }
            };
        };

	///
    /// \brief Common abstract class for LTR-329ALS gains and gain codes.
	///
	/// \details
	///		The LTR-329ALS has an unusual selection of
	///     gain values: 1, 2, 4, 8, then 48 and 96;
    ///     the binary values 16, 32 and 64 are not
    ///     supported.
    ///
	///		To avoid repeating ourselves, we arrange for register image
    ///     subclasses that use gain codes to inherit from this
	///		class. They thereby share the common ability to convert from
	///		numerical gains to gain codes, and vice versa.
    ///
    class AlsGain_t
        {
    public:
        using Gain_t = std::uint8_t;

        /// \brief convert gain value to bits
        static constexpr std::uint8_t gainToBits(Gain_t g)
            {
            return (g == 1)     ? 0
                :  (g == 2)     ? 1
                :  (g == 4)     ? 2
                :  (g == 8)     ? 3
                :  (g == 48)    ? 6
                :  (g == 96)    ? 7
                :               8
                ;
            }

        ///
        /// \brief test whether a gain value is exactly represented and valid.
        ///
        /// \param[in] g    the gain to be checked.
        ///
        /// \details
        ///     This function converts \c g to a gain code and checks that
        ///     it's in range.
        ///
        /// \return \c true if the gain is valid.
        ///
        static constexpr bool isGainValid(uintmax_t g)
            {
            return gainToBits(g) < 8;
            }

        /// \brief return gain bits to numerical gain
        /// \note undefined gain bit values are mapped to gain of 1.
        static constexpr Gain_t bitsToGain(std::uint8_t gbits)
            {
            return  (gbits == 0) ? 1
                :   (gbits == 1) ? 2
                :   (gbits == 2) ? 4
                :   (gbits == 3) ? 8
                :   (gbits == 6) ? 48
                :   (gbits == 7) ? 96
                :                  1
                ;
            }
        };

	///
    /// \brief Register values for the ALS_CONTR register
	///
	/// \details
	///		Values of this type are used to represent images
	///		of values read from or written to the \c ALS_CONTR
	///		register. The methods of this object allow
	///		individual fields to be updated or extracted from
	///		an image.
	///
    ///     A typical way to construct a register value is
    ///     to write something like:
    ///
    ///     \code auto x = AlsContr_t(0).setActive(true).setGain(1);
    ///     \endcode
    ///
    class AlsContr_t : public LTR_329ALS_PARAMS, LTR_329ALS_PARAMS::Field_t<std::uint8_t>, AlsGain_t
        {
    private:
        AlsContr_t & setValue(std::uint8_t fmask, std::uint8_t value)
            {
            this->m_value = fieldset(fmask, this->m_value, value);
            return *this;
            }

        std::uint8_t m_value;

    public:
        AlsContr_t(std::uint8_t mask = 0)
            : m_value(mask)
            {}

        /// \brief return register value converted to a std::uint8_t.
        std::uint8_t getValue() const
            {
            return m_value;
            }

        ///
        /// \brief Manipulate the "active mode" bit in an image of the \c ALS_CONTR register.
        ///
        /// \param [in] fActive     \c true for active mode, \c false for standby mode.
        ///
        /// \return a reference to the register image in a form that can be chain manipulated.
        ///
        AlsContr_t & setActive(bool fActive = true)
            {
            this->setValue(std::uint8_t(ALS_CONTR_BITS::MODE), fActive);
            return *this;
            }

        /// \brief get the active mode value in an image of the \c ALS_CONTR register.
        /// \return \c true if active, \c false if in standby.
        constexpr bool getActive() const
            {
            return this->fieldget(std::uint8_t(ALS_CONTR_BITS::MODE), this->m_value);
            }

        /// \brief set the value of the reset bit in an image of the \c ALS_CONTR register.
        AlsContr_t & setReset(bool fReset = true)
            {
            this->setValue(std::uint8_t(ALS_CONTR_BITS::RESET), fReset);
            return *this;
            }
        /// \brief extract value of the reset bit from an image of the \c ALS_CONTR register.
        constexpr bool getReset() const
            {
            return this->fieldget(std::uint8_t(ALS_CONTR_BITS::RESET), this->m_value);
            }

		///
        /// \brief Set the gain bits in an image of the \c ALS_CONTR register.
        ///
        /// \param [in] g is the gain, in [1, 2, 4, 8, 48, 96]
		///
		/// If the value of \c g is not supported, the gain bits in the register
		///	image are set to select gain == 1.
		///
        AlsContr_t &setGain(std::uint8_t g)
            {
            this->setValue(std::uint8_t(ALS_CONTR_BITS::GAIN), this->gainToBits(g) & 7);
            return *this;
            }

        /// \brief Extract the gain from an image of the \c ALS_CONTR register.
        ///
        /// \return 1, 2, 4, 8, 48, or 96, depending on the value of bits 4:2.
        /// \note If the gain bits are not valid, the returned gain is one.
        constexpr std::uint8_t getGain() const
            {
            return this->bitsToGain(this->fieldget(std::uint8_t(ALS_CONTR_BITS::GAIN), this->m_value));
            }
        };

	///
    /// \brief Register values for the \c ALS_MEAS_RATE register
	///
	/// \details
	///		Values of this type are used to represent images
	///		of values read from or written to the \c ALS_MEAS_RATE
	///		register. The methods of this object allow
	///		individual fields to be updated or extracted from
	///		an image.
	///
    ///     A typical way to construct a register value is
    ///     to write something like:
    ///
    ///     \code
    ///     auto x = AlsMeasRate_t(0)
    ///                 .setRate(1000)
    ///                 .setIntegration(100)
    ///                 ;
    ///     \endcode
    ///
    class AlsMeasRate_t : public LTR_329ALS_PARAMS, LTR_329ALS_PARAMS::Field_t<std::uint8_t>
        {
    private:
        AlsMeasRate_t & setValue(std::uint8_t fmask, std::uint8_t value)
            {
            this->m_value = fieldset(fmask, this->m_value, value);
            return *this;
            }

        std::uint8_t m_value;

    public:
        AlsMeasRate_t(std::uint8_t mask = 0)
            : m_value(mask)
            {}

        /// \brief return register value as a std::uint8_t
        constexpr std::uint8_t getValue() const
            {
            return m_value;
            }

        ///
        /// \brief Abstract type wide enough to store any measurement rate value.
        ///
        /// \note The name is confusing. A rate is usually expressed as per-second, but
        ///     the LTR-329ALS datasheet expresses it in milliseconds (seconds-per).
        ///
        using Rate_t = std::uint16_t;

        /// \brief Abstract type wide enough to store any integration time value.
        using Integration_t = std::uint16_t;

        /// \brief convert measurement rate (expressed as ms / measurement) to register bit value
        constexpr static std::uint8_t rateToBits(Rate_t rate)
            {
            return  (rate <= 50)    ? 0b000
                :   (rate <= 100)   ? 0b001
                :   (rate <= 200)   ? 0b010
                :   (rate <= 500)   ? 0b011
                :   (rate <= 1000)  ? 0b100
                :                     0b101
                ;
            }

        /// \brief convert register bit value to measurement rate in ms
        constexpr static Rate_t bitsToRate(std::uint8_t bits)
            {
            return  (bits == 0b000) ? 50
                :   (bits == 0b001) ? 100
                :   (bits == 0b010) ? 200
                :   (bits == 0b011) ? 500
                :   (bits == 0b100) ? 1000
                :   (0b101 <= bits && bits <= 0b111) ? 2000
                :                     500
                ;
            }

        /// \brief test whether a given rate is valid
        constexpr bool static isRateValid(uintmax_t rate)
            {
            return bitsToRate(rateToBits(rate)) == rate;
            }

        /// \brief convert integration time (in ms) to bits
        constexpr static std::uint8_t integrationToBits(Integration_t iTime)
            {
            return  (iTime <= 50)   ? 0b001
                :   (iTime <= 100)  ? 0b000
                :   (iTime <= 150)  ? 0b100
                :   (iTime <= 200)  ? 0b010
                :   (iTime <= 250)  ? 0b101
                :   (iTime <= 300)  ? 0b110
                :   (iTime <= 350)  ? 0b111
                :   (iTime <= 400)  ? 0b011
                :                     0b000
                ;
            }

        /// \brief convert bits to integration time (in ms)
        constexpr static Integration_t bitsToIntegration(std::uint8_t bits)
            {
            return  (bits == 0b000) ? 100
                :   (bits == 0b001) ? 50
                :   (bits == 0b010) ? 200
                :   (bits == 0b011) ? 400
                :   (bits == 0b100) ? 150
                :   (bits == 0b101) ? 250
                :   (bits == 0b110) ? 300
                :   (bits == 0b111) ? 350
                :                     100
                ;
            }

        /// \brief check that a given integration time is valid
        constexpr static bool isIntegrationValid(std::uintmax_t iTime)
            {
            return bitsToIntegration(integrationToBits(iTime)) == iTime;
            }

        /// \brief the ordered list of integration times
        constexpr static Integration_t vTimes[] = { 50, 100, 150, 200, 250, 300, 350, 400 };

        /// \brief set the measurement rate
        AlsMeasRate_t & setRate(Rate_t rate)
            {
            this->m_value = this->fieldset(std::uint8_t(ALS_MEAS_RATE_BITS::RATE), this->m_value, this->rateToBits(rate));
            return *this;
            }

        /// \brief get the measurement rate
        constexpr Rate_t getRate() const
            {
            return bitsToRate(this->fieldget(std::uint8_t(ALS_MEAS_RATE_BITS::RATE), this->m_value));
            }

        /// \brief set the integration time
        AlsMeasRate_t & setIntegration(Integration_t iTime)
            {
            this->m_value = this->fieldset(std::uint8_t(ALS_MEAS_RATE_BITS::TIME), this->m_value, this->integrationToBits(iTime));
            return *this;
            }

        /// \brief get the integration time
        constexpr Integration_t getIntegration() const
            {
            return bitsToIntegration(this->fieldget(std::uint8_t(ALS_MEAS_RATE_BITS::TIME), this->m_value));
            }
        };

    /// \brief Part ID register bit manipulations
    class PartID_t : public LTR_329ALS_PARAMS, LTR_329ALS_PARAMS::Field_t<std::uint8_t>
        {
    private:
        PartID_t & setValue(std::uint8_t fmask, std::uint8_t value)
            {
            this->m_value = fieldset(fmask, this->m_value, value);
            return *this;
            }

        std::uint8_t m_value;

    public:
        PartID_t(std::uint8_t mask = 0)
            : m_value(mask)
            {}

        /// \brief return register value
        std::uint8_t getValue() const
            {
            return m_value;
            }

        /// \brief the standard PartID value
        static constexpr std::uint8_t kPartID = 0xA;

        /// \brief the standard revision ID value
        static constexpr std::uint8_t kRevID = 0;

        /// \brief extract the part number ID from the PART_ID register
        constexpr std::uint8_t getPartID() const
            {
            return this->fieldget(std::uint8_t(PART_ID_BITS::PARTNUM), this->m_value);
            }

        /// \brief extract the revision ID from the PART_ID register
        constexpr std::uint8_t getRevID() const
            {
            return this->fieldget(std::uint8_t(PART_ID_BITS::REVID), this->m_value);
            }
        };

    /// \brief Manufacturer ID register
    class ManufacID_t : public LTR_329ALS_PARAMS, LTR_329ALS_PARAMS::Field_t<std::uint8_t>
        {
    private:
        ManufacID_t & setValue(std::uint8_t fmask, std::uint8_t value)
            {
            this->m_value = fieldset(fmask, this->m_value, value);
            return *this;
            }

        std::uint8_t m_value;

    public:
        ManufacID_t(std::uint8_t mask = 0)
            : m_value(mask)
            {}

        /// \brief return register value
        std::uint8_t getValue() const
            {
            return m_value;
            }

        /// \brief the standard Manufacturer ID value
        static constexpr std::uint8_t kManufacID = 0x05;

        /// \brief return the Manufacturer ID from the register image
        constexpr std::uint8_t getManufacID() const
            {
            return this->m_value;
            }
        };

	///
    /// \brief Register values for the \c ALS_STATUS register
	///
	/// \details
	///		Values of this type are used to represent images
	///		of values read from the \c ALS_STATUS
	///		register. The methods of this object allow
	///		individual fields to be extracted from
	///		an image without explicit shifting and masking.
    ///
    class AlsStatus_t : public LTR_329ALS_PARAMS, LTR_329ALS_PARAMS::Field_t<std::uint8_t>, AlsGain_t
        {
    private:
        AlsStatus_t & setValue(std::uint8_t fmask, std::uint8_t value)
            {
            this->m_value = fieldset(fmask, this->m_value, value);
            return *this;
            }

        std::uint8_t m_value;

    public:
        AlsStatus_t(std::uint8_t mask = 0)
            : m_value(mask)
            {}

        /// \brief return register value
        std::uint8_t getValue() const
            {
            return m_value;
            }
        /// \brief set the gain (or leave unchanged)
        /// \param [in] g is the gain, in [1, 2, 4, 8, 48, 96]
        AlsStatus_t &setGain(std::uint8_t g)
            {
            this->setValue(std::uint8_t(ALS_STATUS_BITS::GAIN), this->gainToBits(g) & 7);
            return *this;
            }

        /// \brief extract the gain from the control register image.
        /// \return 1, 2, 4, 8, 48, or 96, depending on the value of bits 4:2.
        /// \note if the gain bits are not valid, the returned gain is one.
        constexpr std::uint8_t getGain() const
            {
            return this->bitsToGain(this->fieldget(std::uint8_t(ALS_STATUS_BITS::GAIN), this->m_value));
            }

        /// \brief set the data status bit in a register image
        constexpr AlsStatus_t &setNew(bool fNew = true)
            {
            this->setValue(std::uint8_t(ALS_STATUS_BITS::NEW), fNew);
            return *this;
            }

        /// \brief get the data status bit from a register image
        bool getNew() const
            {
            return this->m_value & std::uint8_t(ALS_STATUS_BITS::NEW);
            }

        /// \brief set the data valid bit in a register image.
        AlsStatus_t& setValid(bool fValid = true)
            {
            this->setValue(std::uint8_t(ALS_STATUS_BITS::INVALID), ! fValid);
            return *this;
            }

        ///
        /// \brief get the data valid bit from a register image.
        ///
        /// \note the valid bit is zero for valid, non-zero for invalid.
        ///
        bool getValid() const
            {
            return ! (this->m_value & std::uint8_t(ALS_STATUS_BITS::INVALID));
            }
        };

    ///
    /// \brief Simple class for LTR-329ALS data registers.
    ///
    /// \details
    ///     Values of this class represent a measurement.
    ///
    ///     Since we're supposed to read all 4 bytes of registers in one go,
    ///     we group them together in one object, and provide methods
    ///     to make it easy for the caller to find the data buffer and the
    ///     size.
    class DataRegs_t
        {
    private:
        /// \brief the register images in I2C order
        std::uint8_t m_data[4];
        AlsStatus_t m_status;       ///< recorded status register when data was grabbed.
        AlsMeasRate_t m_measrate;   ///< recorded measrate used for grabbing the data


    public:
        /// \brief get the value of channel 0 from the measurement
        std::uint16_t getChan0() const
            {
            return (this->m_data[3] << 8) | this->m_data[2];
            }

        /// \brief get the value of channel 1 from the measurement.
        std::uint16_t getChan1() const
            {
            return (this->m_data[1] << 8) | this->m_data[0];
            }

        /// \brief initialize the data buffer to zeroes.
        void init()
            {
            this->m_data[0] = 0;
            this->m_data[1] = 0;
            this->m_data[2] = 0;
            this->m_data[3] = 0;
            this->m_status = AlsStatus_t(0).setValid(false).setNew(false);
            }

        /// \brief return a pointer to the base of the data buffer
        std::uint8_t *getDataPointer()
            {
            return this->m_data;
            }

        /// \brief return size of the data buffer (in bytes)
        constexpr std::size_t getDataSize() const
            {
            return sizeof(this->m_data);
            }

        /// \brief Save an image of the status register for future computation
        void setStatus(AlsStatus_t status)
            {
            this->m_status = status;
            }

        /// \brief Save an image of the meas/rate register for future computation
        void setMeasRate(AlsMeasRate_t measRate)
            {
            this->m_measrate = measRate;
            }

        /// \brief get the integration time previously saved
        AlsMeasRate_t::Integration_t getIntegrationTime() const
            {
            return this->m_measrate.getIntegration();
            }

        ///
        /// \brief Compute abstract value of lux based on datasheet
        ///
        /// \param [in] ch0 is the measurement for channel 0
        /// \param [in] ch1 is the measurement for channel 1
        /// \param [in] gain is the gain used for the measurement (must be valid)
        /// \param [in] iTime is the integration time in milliseconds
        ///
        /// \return The result is the value in Lux per appendix A of the datasheet.
        ///
        static constexpr float luxComputation(
            std::uint16_t ch0,
            std::uint16_t ch1,
            std::uint32_t gain,
            std::uint32_t iTime
            )
            {
            float const ch01_sum = ch0 + ch1;
            if (ch01_sum == 0.0f)
                return 0.0f;

            float const ratio = ch1 / ch01_sum;

            float result = (ratio < 0.45f) ? (1.7743f * ch0 + 1.1059f * ch1)
                         : (ratio < 0.64)  ? (4.2785f * ch0 - 1.9548f * ch1)
                         : (ratio < 0.85)  ? (0.5926f * ch0 + 0.1185f * ch1)
                         :                   0.0f
                         ;

            return (result * 100.0f) / (gain * iTime);
            }

        ///
        /// \brief Compute lux based on the value of the data stored here
        ///
        /// \param [out] fError reference to a bool cell set true in case of
        ///                     error in the calculation, set false if all
        ///                     went well.
        ///
        /// \return The light value, in lux; or zero.
        ///
        float computeLux(bool &fError) const
            {
            if (! this->m_status.getValid())
                {
                fError = true;
                return 0.0f;
                }
            if (! this->m_status.getNew())
                {
                fError = true;
                return 0.0f;
                }

            float result = this->luxComputation(
                                this->getChan0(),
                                this->getChan1(),
                                this->m_status.getGain(),
                                this->m_measrate.getIntegration()
                                );
            fError = false;
            return result;
            }
        };

    //
    // Testing
    //
    static_assert(AlsMeasRate_t::isRateValid(50), "50ms should be valid");
    static_assert(AlsMeasRate_t::isRateValid(100), "100ms should be valid");
    static_assert(AlsMeasRate_t::isRateValid(1000), "1000ms should be valid");
    static_assert(AlsMeasRate_t::isRateValid(2000), "2000ms should be valid");
    static_assert(! AlsMeasRate_t::isRateValid(9999), "9999ms should be valid");
    static_assert(! AlsMeasRate_t::isRateValid(2000+0x80000u), "big val should not be valid");
    static_assert(! AlsMeasRate_t::isRateValid(10), "10 ms should not be valid");
    static_assert(! AlsMeasRate_t::isRateValid(0), "0 ms should not be valid");

    static_assert(AlsMeasRate_t::isIntegrationValid(50), "50 ms should be valid");
    static_assert(AlsMeasRate_t::isIntegrationValid(100), "100 ms should be valid");
    static_assert(AlsMeasRate_t::isIntegrationValid(150), "150 ms should be valid");
    static_assert(AlsMeasRate_t::isIntegrationValid(200), "200 ms should be valid");
    static_assert(AlsMeasRate_t::isIntegrationValid(250), "250 ms should be valid");
    static_assert(AlsMeasRate_t::isIntegrationValid(300), "300 ms should be valid");
    static_assert(AlsMeasRate_t::isIntegrationValid(350), "350 ms should be valid");
    static_assert(AlsMeasRate_t::isIntegrationValid(400), "400 ms should be valid");
    static_assert(! AlsMeasRate_t::isIntegrationValid(0), "0 ms should not be valid");
    static_assert(! AlsMeasRate_t::isIntegrationValid(49), "49 ms should not be valid");
    static_assert(! AlsMeasRate_t::isIntegrationValid(51), "51 ms should not be valid");
    static_assert(! AlsMeasRate_t::isIntegrationValid(1999), "1999 ms should not be valid");

    static_assert(DataRegs_t::luxComputation(0, 0, 1, 100) == 0.0, "lux computation is wrong");
    static_assert(DataRegs_t::luxComputation(50, 100, 1, 100) != 0.0, "lux computation is wrong");
    static_assert(DataRegs_t::luxComputation(100, 0, 1, 100) == 177.43f, "lux computation is wrong");
    static_assert(DataRegs_t::luxComputation(1000, 100, 4, 200) == 235.6112366f, "lux computation is wrong");
}

#endif /* _mcci_ltr_329als_regs_h_ */
