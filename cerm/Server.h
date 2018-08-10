#pragma once
#include <mutex>
#include <memory>
#include <list>
#include <mutex>
#include "Client.h"
#include "PiReaderService.h"
#include "Configuration.h"
#include "ServerSocket.h"

class Server
{
private:
	Server();
	Server(Server const&) {};//no implementation
	Server& operator=(Server const&) {};//no implementation

	std::list<std::shared_ptr<Client>> _clients; //All connected clients have a smart pointer reference on this list. When client disconnects, this list is cleared by a service.
	std::unique_ptr<PiReaderService> _readerService;
	std::unique_ptr<ServerSocket> _serverSocket;

	std::mutex _clientsListMutex;
	std::shared_ptr<Configuration::CFG_SERVER> _config;
	
	void cleanup(); //this function removes all ClientRequests from the list if isConnected == false.

public:
	~Server();
	static Server * getInstance();
	void run();

	int getNumberOfConnectedClients();

	std::list<std::shared_ptr<Client>> getConnectedClients();
};

