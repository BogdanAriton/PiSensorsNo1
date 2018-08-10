#pragma once
#include <arpa/inet.h>
#include "Client.h"

class RemoteClient : public Client
{
private:
	int _socketDescriptor;

	std::list<uint8_t> _buffer;

	std::chrono::steady_clock::time_point _lastHeartbeat;
	
	//Sends data to a host. If client is not connected / writing error, throws.
	bool sendData(uint8_t data[], short size);

	//Reads data from a client. If no data is available, it returns a vector with 0 elements.
	//If the client is not connected / reading errors, throws.
	//It returns true if it can receive the data, false otherwise
	bool readData(uint8_t data[], short size);
	
	void tick();

public:
	RemoteClient(int descriptor, sockaddr_in cli_addr);
	~RemoteClient();
	
	bool sendMessage(std::string message);

	void addToBuffers(uint8_t valueCH0, uint8_t valueCH1);
};

