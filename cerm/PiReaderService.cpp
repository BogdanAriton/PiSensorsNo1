#include <chrono>
#include <cmath>
#include <wiringPi.h>
#include "PiReaderService.h"
#include "Server.h"
#include "ADS1015Driver.h"
#include "Logger.h"

using namespace std;
using namespace std::chrono_literals;

PiReaderService::PiReaderService()
{
	Logger::getInstance()->logTrace("PiReaderService constructor called.");
	_config = Configuration::getInstance()->getConfiguration()->DataReader;
	
	_isWorkerActive = true;

	_driver = std::make_unique<ADS1015Driver>();
	_worker = std::make_unique<std::thread>(&PiReaderService::doWork, this);
	
	_userInterface = std::make_unique<UserInterface>();
	_userInterface->runAsync();

	wiringPiSetupGpio();

	pinMode(pinPLOSLow, INPUT);
	pinMode(pinMINUSLow, INPUT);

	Logger::getInstance()->logTrace("PiReaderService instance created.");
}

PiReaderService::~PiReaderService()
{
	Logger::getInstance()->logTrace("PiReaderService destructor called!");

	_isWorkerActive = false;
	_worker->join();
	
	Logger::getInstance()->logTrace("PiReaderService instance destroyed.");
}

void PiReaderService::doWork()
{
	while (_isWorkerActive)
	{
		auto clients = Server::getInstance()->getConnectedClients();//copy the list with shared_ptr in it.
		
		if (Server::getInstance()->getNumberOfConnectedClients() > 0 && _userInterface->isSystemPaused())
		{
			_userInterface->togglePause();
		}
		else if (Server::getInstance()->getNumberOfConnectedClients() == 0 && !_userInterface->isSystemPaused())
		{
			_userInterface->togglePause();
		}

		if (clients.size() > 0) //Will always work if the GUI is enabled.
		{
			//Scaling everything to 8 bits from 12 bits.
			uint8_t ecgValue = nearbyint((_driver->getValue(0, static_cast<ADS1015Driver::GAIN>(_config->_gainEKG))) * (255.f / 4096.f));
			uint8_t hbValue = nearbyint((_driver->getValue(1, static_cast<ADS1015Driver::GAIN>(_config->_gainHB))) * (255.f / 4096.f));
			//Uncomment this to show time between reads.

			if (areECGLeadsDisconnected()) //Used this to ensure reading datarate.
			{
				ecgValue = 0;
			}

			_userInterface->addToBuffers(ecgValue, hbValue);

			for (auto & client : clients)
			{
				if (client->isClientConnected() && client->isClientRequestingData())
				{
					client->addToBuffers(ecgValue,hbValue);
				}
			}
		}
		else
		{
			std::this_thread::sleep_for(100ms); //If no client is connected, sleep for 100ms then try again to send data.
		}
	}
}

bool PiReaderService::areECGLeadsDisconnected()
{
	return (digitalRead(pinPLOSLow) || digitalRead(pinMINUSLow)); //If 20 or 21 is low returns False
}