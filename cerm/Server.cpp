#include <algorithm>
#include <string.h>
#include "Server.h"
#include "ServerSocket.h"
#include "RemoteClient.h"
#include "Logger.h"

using namespace std::chrono_literals;

Server::Server()
{
	_config = Configuration::getInstance()->getConfiguration()->Server;

	_readerService = std::make_unique<PiReaderService>();
	_serverSocket = std::make_unique<ServerSocket>(_config->_port); //Creating the ServerSocket with the port and IOC

	std::thread GC(&Server::cleanup, this);
	GC.detach();

	Logger::getInstance()->logInfo("Server created!");
}

Server::~Server()
{
	Logger::getInstance()->logInfo("Server shutting down!");
}

Server * Server::getInstance()
{
	static Server server; //Strange indeed, but the standard guarantees this is thread safe.
	return &server; //TODO change this to smart pointer.
}

void Server::run()
{
	Logger::getInstance()->logInfo("Awaiting for requests...");

	while (true)
	{
		auto request = _serverSocket->acceptConnection();

		if (request == nullptr)
			continue;

		request->runAsync();

		std::lock_guard<std::mutex> lg(_clientsListMutex);

		if (_clients.size() < _config->_maxConcurrentClients)
		{
			_clients.push_back(request);
		}
		else
		{
			request->sendMessage("YOU SHALL NOT CONNECT AS THERE IS NO PLACE FOR YOU IN THE QUEUE!");
		}
	}
}

int Server::getNumberOfConnectedClients()
{
	std::lock_guard<std::mutex> lg(_clientsListMutex);
	int nr = 0;

	std::for_each(_clients.begin(), _clients.end(), [&nr](auto it)
	{
		if (it->isClientConnected())
			nr++;
	});

	return nr;
}

void Server::cleanup()
{
	while (true)
	{
		std::this_thread::sleep_for(_config->_serverCleanupDeltaInterval); //GC every x seconds.

		std::lock_guard<std::mutex> lg(_clientsListMutex);

		auto it = std::remove_if(_clients.begin(), _clients.end(), [](const std::shared_ptr<Client> & element)
		{
			return !element->isClientConnected();
		});

		_clients.erase(it, _clients.end());

		Logger::getInstance()->logInfo("Server cleaned!");
	}
}

std::list<std::shared_ptr<Client>> Server::getConnectedClients()
{
	//This function returns a copy of _clients. The pointers won't be deleted by the clanup until the copyer is done with them.
	//The lock will be released when lg gets out of scope.
	std::lock_guard<std::mutex> lg(_clientsListMutex);
	return _clients;
}
