#include <wiringPiI2C.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>
#include <sstream>
#include "ADS1015Driver.h"
#include "Logger.h"

#define ADS1015_REG_POINTER_MASK        (0x03)
#define ADS1015_REG_POINTER_CONVERT     (0x00)
#define ADS1015_REG_POINTER_CONFIG      (0x01)
#define ADS1015_REG_POINTER_LOWTHRESH   (0x02)
#define ADS1015_REG_POINTER_HITHRESH    (0x03)

ADS1015Driver::ADS1015Driver()
{
	_config = Configuration::getInstance()->getConfiguration()->DataReader;
	uint8_t  m_i2cAddress = _config->_I2CDigitalConverterAddress;
	DRATE drate = static_cast<ADS1015Driver::DRATE>(_config->_dataRate);

	std::stringstream stream;
	stream << "ADS1015Driver constructor called with params : " << "I2C : " << std::to_string(m_i2cAddress) << " " << "Data Rate : " << drate;

	Logger::getInstance()->logTrace(stream.str());

	_descriptor = wiringPiI2CSetup(m_i2cAddress); //Creates the standard linux descriptor.

	switch (drate)
	{
	case D128:
		_dRateValue = 128;
		_dRateMask = 0x0000;
		break;
	case D250:
		_dRateValue = 250;
		_dRateMask = 0x0020;
		break;
	case D490:
		_dRateValue = 490;
		_dRateMask = 0x0040;
		break;
	case D920:
		_dRateValue = 920;
		_dRateMask = 0x0060;
		break;
	case D1600:
		_dRateValue = 1600;
		_dRateMask = 0x0080;
		break;
	case D2400:
		_dRateValue = 2400;
		_dRateMask = 0x00A0;
		break;
	case D3300:
		_dRateValue = 3300;
		_dRateMask = 0x00C0;
		break;
	}
	
	Logger::getInstance()->logTrace("ADS1015 driver created.");
}

ADS1015Driver::~ADS1015Driver()
{
	Logger::getInstance()->logTrace("ADS1015 destructor called.");
}

uint16_t ADS1015Driver::getValue(uint8_t channel, GAIN gain)
{
	if (channel < 0 || channel > 4)
	{
		throw;
	}

	channel += 4;//Channel offset

	uint16_t config = _default_config;

	config |= 0x8000; //Single shot mode, must be set every time we require a conversion.
	config |= (channel & 0x07) << 12; //Set channel
	config |= static_cast<typename std::underlying_type<GAIN>::type>(gain); //Set gain amplifier, gets enum value
	config |= _dRateMask; //Set data rate
	
	wiringPiI2CWriteReg16(_descriptor, ADS1015_REG_POINTER_CONFIG, ntohs(config)); //Writing configuration to the register.

	std::this_thread::sleep_for(std::chrono::microseconds((int)((float)(1.f / _dRateValue) * 1000 * 1000) + 100)); //Conversion, seconds to microseconds

	uint16_t data = ntohs(wiringPiI2CReadReg16(_descriptor, ADS1015_REG_POINTER_CONVERT)); // Reversing the network bit order to host bit order.

	return data >> 4; //Last 4 bits are 0.
}
