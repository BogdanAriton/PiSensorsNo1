#pragma once
#include "Configuration.h"

///Use this class to read from the ADS.
///Author : Cozminca Emilian, CERNER intern, 25.08.2017
class ADS1015Driver
{
public:
	///The gain amplifier. If the analog devices gives an output between 0-5V, then we chose 6.144.
	///The gain should be the the first value above the maximum output of the device.
	///If the gain is too low or too high, no useful data is obtained.
	///Codification : G - gain, D - decimal point. Ex : G6D144 means +6.144 V
	enum GAIN {
		G6D144 = 0,
		G4D096 = 1,
		G2D048 = 2,
		G1D024 = 3,
		G0D512 = 4,
		G0D256 = 5
	};

	///This value specifies the data rate the device converts. If we set 3300, then we can read 3300 values in a second.
	enum DRATE {
		D128 = 0,
		D250 = 1,
		D490 = 2,
		D920 = 3,
		D1600 = 4,
		D2400 = 5,
		D3300 = 6
	};

	///The default address of the ADS1015 is 0x48 and the data rate can be:
	ADS1015Driver();
	~ADS1015Driver();

	std::shared_ptr<Configuration::CFG_DATA_READER> _config;

	///This method returns the last converted value from the device. The channel can range from 0 to 3.
	///This function blocks for (1.0 / dRateValue) * 1000 microseconds.
	uint16_t getValue(uint8_t channel, GAIN gain);

private:
	int _descriptor; //Standard linux file descriptor.
	uint16_t _dRateValue; //Used for sleep.
	uint16_t  _dRateMask; //Used for config OR operator.
	const uint16_t _default_config = 0b0000001110000011; //Config register. For more info, visit https://cdn-shop.adafruit.com/datasheets/ads1015.pdf
};