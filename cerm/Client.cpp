#include <inttypes.h>
#include <iostream>	
#include "Client.h"
#include "Logger.h"

using namespace std::chrono_literals;

Client::Client()
{
	_isClientConnected = true;
	_isClientRequestingData = false;
}
Client::~Client() {}

bool Client::isClientConnected()
{
	return _isClientConnected;
}

bool Client::isClientRequestingData()
{
	return _isClientRequestingData;
}

void Client::runAsync()
{
	_worker = std::make_unique<std::thread>(std::thread([this]()
	{
		while (this->_isClientConnected)
		{
			this->tick();
			std::this_thread::sleep_for(1ms);
		}
		Logger::getInstance()->logTrace("Client disconnected.");
	}
	));
}
