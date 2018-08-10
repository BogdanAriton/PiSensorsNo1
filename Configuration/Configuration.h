#pragma once
#include <memory>
#include "INIReader.h"

#define CFG_VALUES_MIN 0
#define LOGGER_LOG_LEVEL_MAX 4
#define DATA_READER_DATA_RATE_MAX 6
#define DATA_READER_GAIN_MAX 5
#define GRAPH_RES_MAX 5

class Configuration
{
public:
	struct CFG_SERVER {
		uint16_t _port = 27015;
		uint8_t _maxConcurrentClients = 4;
		std::chrono::seconds _clientTimeoutDeltaInterval = std::chrono::seconds(15);
		std::chrono::seconds _serverCleanupDeltaInterval = std::chrono::seconds(15);
	};

	struct CFG_LOGGER {
		uint8_t _logLevel = 0;
		bool _logToFile = true;
		bool _logToConsole = true;
		bool _logToDatabase = false;
		std::string _fileExtension = "txt";
		float _maxFileSize = 5.f;
	};

	struct CFG_DATA_READER {
		uint8_t _I2CDigitalConverterAddress = 0x48;
		uint8_t _dataRate = 6;
		uint8_t _gainEKG = 1;
		uint8_t _gainHB = 1;
	};

	struct CFG_SIGNAL_SIMULATOR {
		std::chrono::milliseconds _signalDeltaInterval = std::chrono::milliseconds(1);
		float _iteratorSkip = 0.01f;
	};

	struct CFG_GRAPH {
		bool _realtimeGraph = true;
		bool _showStats = true;
	};

	struct CONFIGURATION {
		//Initilized default values in case file is missing and should be created;	
		std::shared_ptr<CFG_SERVER> Server;
		std::shared_ptr<CFG_LOGGER> Logger;
		std::shared_ptr<CFG_DATA_READER> DataReader;
		std::shared_ptr<CFG_SIGNAL_SIMULATOR> SignalSimulator;
		std::shared_ptr<CFG_GRAPH> Graph;

		CONFIGURATION();
	};
	
	static Configuration * getInstance();
	std::shared_ptr<CONFIGURATION> getConfiguration();

private:
	const std::string _configPath = "configuration.ini";
	std::shared_ptr<CONFIGURATION> _config;

	Configuration();
	Configuration(Configuration const&) {};//no implementation
	Configuration& operator=(Configuration const&) {};//no implementation

	void createConfigurationFile();
	void setConfigurationFromFile(INIReader reader);
};