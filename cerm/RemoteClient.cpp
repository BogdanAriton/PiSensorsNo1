#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <vector>
#include <string.h>
#include <arpa/inet.h>
#include "RemoteClient.h"
#include "Logger.h"
#include "Configuration.h"

RemoteClient::RemoteClient(int descriptor, sockaddr_in cli_addr) :_socketDescriptor(descriptor)
{
	//Log connected client IP
	char msg[50] = "Client connected ! Address : ";
	char addr[16];
	inet_ntop(AF_INET, &(cli_addr.sin_addr), addr, INET_ADDRSTRLEN);

	strcat(msg, addr);

	Logger::getInstance()->logInfo(msg);
	//~Log IP

	_isClientRequestingData = false;
	_isClientConnected = true;

	_lastHeartbeat = std::chrono::steady_clock::now();
}

RemoteClient::~RemoteClient()
{
	_isClientConnected = false;

	_worker->join();

	close(_socketDescriptor);

	Logger::getInstance()->logInfo("Client purged!");
}

void RemoteClient::tick()
{
	uint8_t recCommand[1];

	if (readData(recCommand, 1))
	{
		if (recCommand[0] == '0')
		{
			_isClientRequestingData = false;
			Logger::getInstance()->logInfo("Remote client is requesting data - false");
		}
		else if (recCommand[0] == '1')
		{
			_isClientRequestingData = true;
			Logger::getInstance()->logInfo("Remote client is requesting data - true");

		}
		_lastHeartbeat = std::chrono::steady_clock::now();
	}

	if (_isClientRequestingData)
	{
		uint8_t *data = nullptr;
		int dataSize = 0;

		_mutexChannels.lock();

		if (_buffer.size() >= 100)
		{
			dataSize = _buffer.size();

			data = new uint8_t[dataSize];

			std::copy(_buffer.begin(), _buffer.end(), data);

			_buffer.clear();
		}

		_mutexChannels.unlock();

		if (dataSize != 0)
		{
			if (!sendData(data, dataSize))
			{
				_isClientConnected = false;
				Logger::getInstance()->logInfo("Remote client data write error! Client will now be disconnected.");
			}
		}
		
		delete data; //will be deleted even if nullptr.

	}

	auto now = std::chrono::steady_clock::now();

	if (std::chrono::duration_cast<std::chrono::seconds>(now - _lastHeartbeat).count() > Configuration::getInstance()->getConfiguration()->Server->_clientTimeoutDeltaInterval.count())  //Hardcoded 15 sec no activity
	{
		_isClientConnected = false;
		Logger::getInstance()->logInfo("Remote client timed out.");
	}
}

//The size is the number of bytes to read.
bool RemoteClient::readData(uint8_t data[], short size)
{
	int count;
	ioctl(_socketDescriptor, FIONREAD, &count);//Checks the number of bytes available on the socket.

	if (count < size)
	{
		return false;
	}

	read(_socketDescriptor, data, size);

	return true;
}

bool RemoteClient::sendData(uint8_t data[], short size)
{
	if (send(_socketDescriptor, data, size, MSG_NOSIGNAL) == -1)
	{
		return false;
	}
	return true;
}

bool RemoteClient::sendMessage(std::string message)
{
	if (send(_socketDescriptor, message.data(), message.size(), MSG_NOSIGNAL) == -1)
	{
		return false;
	}
	return true;
}

void RemoteClient::addToBuffers(uint8_t valueCH0, uint8_t valueCH1)
{
	if (_isClientConnected && _isClientRequestingData)
	{
		std::lock_guard<std::mutex> lg(_mutexChannels);
		_buffer.push_back(valueCH0);
		_buffer.push_back(valueCH1);
	}
}