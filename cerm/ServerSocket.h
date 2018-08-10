#pragma once
#include "RemoteClient.h"

class ServerSocket
{
private:
	int _socketDescriptor; //Server socket descriptor.

public:
	ServerSocket(uint16_t port);
	~ServerSocket();

	std::shared_ptr<RemoteClient> acceptConnection();	//To be used in the server loop. It accepts a connection, returns a shared_ptr<ClientRequest> 
};