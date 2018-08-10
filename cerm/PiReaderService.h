#pragma once
#include <thread>
#include <memory>
#include <atomic>
#include "UserInterface.h"
#include "Configuration.h"

//Prevents cyclic reference.
class Server;
class ADS1015Driver;

///This class is used to get a response from the server. It requires a Server because we need to check how many clients are connected.
///If no clients are conected, no update occurs. It awaits 200 ms the checks again.
///If a client is connected, it updates the ECG and HB values.
///It reads 2 values with a speed of 1600 values/second, ~800 updates/second for each server.
class PiReaderService
{
private:
	const uint8_t pinMINUSLow = 21;
	const uint8_t pinPLOSLow = 20;

	bool areECGLeadsDisconnected();

	std::shared_ptr<Configuration::CFG_DATA_READER> _config;

	std::unique_ptr<ADS1015Driver> _driver;
	std::unique_ptr<std::thread> _worker;
	std::unique_ptr<UserInterface> _userInterface;
	std::atomic<bool> _isWorkerActive;
	
	void doWork();

public:
	PiReaderService();
	~PiReaderService();
};

