#include <fstream>
#include <chrono>
#include "Configuration.h"

using namespace std;

Configuration::Configuration()
{
	_config = std::make_shared<CONFIGURATION>();

	INIReader reader(_configPath);
	//Create configuration file with default values if it doesn't exist
	if (reader.ParseError() < 0) 
	{
		createConfigurationFile();
	}
	//Retrieve data from configuration file
	else 
	{
		setConfigurationFromFile(reader);
	}
}

std::shared_ptr<Configuration::CONFIGURATION> Configuration::getConfiguration()
{
	return _config;
}

Configuration::CONFIGURATION::CONFIGURATION()
{
	Server = std::make_shared<CFG_SERVER>();
	Logger = std::make_shared<CFG_LOGGER>();
	DataReader = std::make_shared<CFG_DATA_READER>();
	SignalSimulator = std::make_shared<CFG_SIGNAL_SIMULATOR>();
	Graph = std::make_shared<CFG_GRAPH>();
}

Configuration * Configuration::getInstance()
{
	static Configuration config; //Strange indeed, but the standard guarantees this is thread safe.
	return &config;
}

void Configuration::createConfigurationFile()
{
	std::ofstream file;
	file.open(_configPath);

	file << "[Server]" << endl
		<< "port = " << to_string(_config->Server->_port) << " ;Default = 27015" << endl
		<< "maxConcurrentClients = " << to_string(_config->Server->_maxConcurrentClients) << " ;Default = 4" << endl
		<< "clientTimeoutDeltaInterval = " << _config->Server->_clientTimeoutDeltaInterval.count() << " ;Default = 15 seconds" << endl
		<< "serverCleanupDeltaInterval = " << _config->Server->_serverCleanupDeltaInterval.count() << " ;Default = 15 seconds" << endl << endl

		<< "[Logger]" << endl
		<< "logLevel = " << to_string(_config->Logger->_logLevel) << " ;Trace = 0, Info = 1, Debug = 2, Warning = 3, Error = 4" << endl
		<< "logToFile = " << _config->Logger->_logToFile << " ;Default true = 1" << endl
		<< "logToConsole = " << _config->Logger->_logToConsole << " ;Default true = 1" << endl
		<< "logToDatabase = " << _config->Logger->_logToDatabase << " ;Default false = 0" << endl
		<< "fileExtension = " << _config->Logger->_fileExtension << " ;Default = txt" << endl
		<< "maxFileSize = " << _config->Logger->_maxFileSize << " ;Size in Megabytes, Default = 5Mb" << endl << endl

		<< "[Data Reader]" << endl
		<< "I2CDigitalConverterAddress = " << to_string(_config->DataReader->_I2CDigitalConverterAddress) << " ;Default = 72 (0x48)" << endl
		<< "dataRate = " << to_string(_config->DataReader->_dataRate) << " ;D128 = 0, D250 = 1, D490 = 2, D920 = 3, D1600 = 4, D2400 = 5, D3300 = 6, Default = 6" << endl
		<< "gainEKG " << to_string(_config->DataReader->_gainEKG) << " ;G6D144 = 0, G4D096 = 1, G2D048 = 2, G1D024 = 3, G0D512 = 4, G0D256 = 5, Default = 1" << endl
		<< "gainHB = " << to_string(_config->DataReader->_gainHB) << " ;G6D144 = 0, G4D096 = 1, G2D048 = 2, G1D024 = 3, G0D512 = 4, G0D256 = 5, Default = 1" << endl << endl

		<< "[Signal Simulator]" << endl
		<< "signalDeltaInterval = " << _config->SignalSimulator->_signalDeltaInterval.count() << ";Value in milliseconds, Default = 1" << endl
		<< "iteratorSkip = " << _config->SignalSimulator->_iteratorSkip << " ;Default = 0.01" << endl << endl

		<< "[Graph]" << endl
		<< "realtimeGraph = " << _config->Graph->_realtimeGraph << " ;Default true = 1" << endl
		<< "showStats = " << _config->Graph->_showStats << " ;Default true = 1" << endl;
	file.close();
}

void Configuration::setConfigurationFromFile(INIReader reader)
{
	//Retrieve Server configuration data with default values if config file key/values are missing or out of range
	//Port range - >0
	_config->Server->_port = (reader.GetInteger("Server", "port", _config->Server->_port) < CFG_VALUES_MIN ? _config->Server->_port :
		reader.GetInteger("Server", "port", _config->Server->_port));
	//Max concurreent clients range - >0
	_config->Server->_maxConcurrentClients = (reader.GetInteger("Server", "maxConcurrentClients", _config->Server->_maxConcurrentClients) < CFG_VALUES_MIN
		? _config->Server->_maxConcurrentClients : reader.GetInteger("Server", "maxConcurrentClients", _config->Server->_maxConcurrentClients));
	//Client timeout interval - >0 seconds
	_config->Server->_clientTimeoutDeltaInterval = std::chrono::seconds(
		reader.GetInteger("Server", "clientTimeoutDeltaInterval", _config->Server->_clientTimeoutDeltaInterval.count()) < CFG_VALUES_MIN ?
		_config->Server->_clientTimeoutDeltaInterval.count() :
		reader.GetInteger("Server", "clientTimeoutDeltaInterval", _config->Server->_clientTimeoutDeltaInterval.count()));
	//Server cleanup interval - >0 seconds
	_config->Server->_serverCleanupDeltaInterval = std::chrono::seconds(
		reader.GetInteger("Server", "serverCleanupDeltaInterval", _config->Server->_serverCleanupDeltaInterval.count()) < CFG_VALUES_MIN ?
		_config->Server->_serverCleanupDeltaInterval.count() :
		reader.GetInteger("Server", "serverCleanupDeltaInterval", _config->Server->_serverCleanupDeltaInterval.count()));

	//Retrieve Logger configuration data with default values if config file key/values are missing or out of range
	//Log Level range - 0->4
	_config->Logger->_logLevel = ((reader.GetInteger("Logger", "logLevel", _config->Logger->_logLevel) < CFG_VALUES_MIN ||
		reader.GetInteger("Logger", "logLevel", _config->Logger->_logLevel) > LOGGER_LOG_LEVEL_MAX) ? _config->Logger->_logLevel :
		reader.GetInteger("Logger", "logLevel", _config->Logger->_logLevel));
	//Log to File range - (true,yes,on,1)/(false,no,off,0)
	_config->Logger->_logToFile = reader.GetBoolean("Logger", "logToFile", _config->Logger->_logToFile);
	//Log to Console range - (true,yes,on,1)/(false,no,off,0)
	_config->Logger->_logToConsole = reader.GetBoolean("Logger", "logToConsole", _config->Logger->_logToConsole);
	//Log to Database range - (true,yes,on,1)/(false,no,off,0)
	_config->Logger->_logToDatabase = reader.GetBoolean("Logger", "logToDatabase", _config->Logger->_logToDatabase);
	//File exntension range - anything, it's a string
	_config->Logger->_fileExtension = reader.GetString("Logger", "fileExtension", _config->Logger->_fileExtension);
	//Max file size range - >0Mb
	_config->Logger->_maxFileSize = (reader.GetReal("Logger", "maxFileSize", _config->Logger->_maxFileSize) < CFG_VALUES_MIN ?
		_config->Logger->_logLevel : reader.GetReal("Logger", "maxFileSize", _config->Logger->_maxFileSize));

	//Retrieve Data Reader configuration data with default values if config file key/values are missing or out of range
	//I2C Address range - >0
	_config->DataReader->_I2CDigitalConverterAddress = (reader.GetInteger("Data Reader", "I2CDigitalConverterAddress", _config->DataReader->_I2CDigitalConverterAddress) < CFG_VALUES_MIN
		? _config->DataReader->_I2CDigitalConverterAddress : reader.GetInteger("Data Reader", "I2CDigitalConverterAddress", _config->DataReader->_I2CDigitalConverterAddress));
	//Data rate range - 0->5
	_config->DataReader->_dataRate = ((reader.GetInteger("Data Reader", "dataRate", _config->DataReader->_dataRate) < CFG_VALUES_MIN ||
		reader.GetInteger("Data Reader", "dataRate", _config->DataReader->_dataRate) > DATA_READER_DATA_RATE_MAX) ? _config->DataReader->_dataRate :
		reader.GetInteger("Data Reader", "dataRate", _config->DataReader->_dataRate));
	//Gain EKG range - 0->4
	_config->DataReader->_gainEKG = ((reader.GetInteger("Data Reader", "gainEKG", _config->DataReader->_gainEKG) < CFG_VALUES_MIN ||
		reader.GetInteger("Data Reader", "gainEKG", _config->DataReader->_gainEKG) > DATA_READER_GAIN_MAX) ? _config->DataReader->_gainEKG :
		reader.GetInteger("Data Reader", "gainEKG", _config->DataReader->_gainEKG));
	//Gain HB range - 0->4
	_config->DataReader->_gainHB = ((reader.GetInteger("Data Reader", "gainHB", _config->DataReader->_gainHB) < CFG_VALUES_MIN ||
		reader.GetInteger("Data Reader", "gainHB", _config->DataReader->_gainHB) > DATA_READER_GAIN_MAX) ? _config->DataReader->_gainHB :
		reader.GetInteger("Data Reader", "gainHB", _config->DataReader->_gainHB));

	//Retrieve Signal Simulator configuration data with default values if config file key/values are missing or out of range
	//Signal delta interval range - >0 milliseconds
	_config->SignalSimulator->_signalDeltaInterval = std::chrono::milliseconds(reader.GetInteger("Signal Simulator", "signalDeltaInterval", _config->SignalSimulator->_signalDeltaInterval.count())
		< CFG_VALUES_MIN ? _config->SignalSimulator->_signalDeltaInterval.count() : reader.GetInteger("Signal Simulator", "signalDeltaInterval", _config->SignalSimulator->_signalDeltaInterval.count()));
	//Iterator skip range - >0
	_config->SignalSimulator->_iteratorSkip = (reader.GetReal("Signal Simulator", "iteratorSkip", _config->SignalSimulator->_iteratorSkip) < CFG_VALUES_MIN ?
		_config->SignalSimulator->_iteratorSkip : reader.GetReal("Signal Simulator", "iteratorSkip", _config->SignalSimulator->_iteratorSkip));

	//Retrieve Graph configuration data with default values if config file key/values are missing or out of range
	//Realtime graph range - (true,yes,on,1)/(false,no,off,0)
	_config->Graph->_realtimeGraph = reader.GetBoolean("Graph", "realtimeGraph", _config->Graph->_realtimeGraph);
	//Show stats range - (true,yes,on,1)/(false,no,off,0)
	_config->Graph->_showStats = reader.GetBoolean("Graph", "showStats", _config->Graph->_showStats);

	//Retrieve Filters configuration data with default values if config file key/values are missing or out of range
	//TO DO ?

	//Retrieve Database configuration data with default values if config file key/values are missing or out of range
	//TO DO ?

	//TO DO ? Update configuration file if key/values are missing or out of range and set them to default
}
