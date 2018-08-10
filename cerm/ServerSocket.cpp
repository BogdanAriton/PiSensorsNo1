#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>

#include "ServerSocket.h"
#include "RemoteClient.h"
#include "Server.h"
#include "Logger.h"

using namespace std;
using namespace std::chrono_literals;

ServerSocket::ServerSocket(uint16_t port) 
{
	Logger::getInstance()->logTrace("ServerSocket constructor called.");
	_socketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//Creates a TCP connexion.

	if (_socketDescriptor < 0)
	{
		Logger::getInstance()->logError("Can't create socket! : " + port);
		exit(-1); //Nothing can be done, log the error and pray a dev sees it.
	}

	sockaddr_in _serv_addr;
	bzero((char *)&_serv_addr, sizeof(_serv_addr));// Initializes the struct with 0'

	_serv_addr.sin_family = AF_INET;
	_serv_addr.sin_addr.s_addr = INADDR_ANY;
	_serv_addr.sin_port = htons(port);

	if (bind(_socketDescriptor, (struct sockaddr *) &_serv_addr, sizeof(_serv_addr)) < 0)
	{
		Logger::getInstance()->logError("Can't bind! " + errno);
		exit(-1); 
	}

	//fcntl(_socketDescriptor, F_SETFL, fcntl(_socketDescriptor, F_GETFL, 0) | O_NONBLOCK);

	listen(_socketDescriptor, Configuration::getInstance()->getConfiguration()->Server->_maxConcurrentClients);
}

std::shared_ptr<RemoteClient> ServerSocket::acceptConnection()
{
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	clilen = sizeof(cli_addr);

	int clientSocketDescriptor = accept(_socketDescriptor, (struct sockaddr *) &cli_addr, &clilen);

	if (clientSocketDescriptor < 0) //Non-blocking socket, returns -1 if no client awaits.
	{
		//std::this_thread::sleep_for(500ms);
		//Something whent wrong while connecting to the client. No need to log it, but we must handle that case in the while loop.
		return nullptr;
	}
	else
	{
		return std::make_shared<RemoteClient>(clientSocketDescriptor, cli_addr); //No copy constructor is invoked. Instead, a move constructor is used.(Unique ptr)
	}
}

ServerSocket::~ServerSocket()
{
	Logger::getInstance()->logTrace("ServerSocket destructor called.");
	close(_socketDescriptor);
}