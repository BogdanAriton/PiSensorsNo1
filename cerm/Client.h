#pragma once
#include <list>
#include <mutex>
#include <thread>
#include <atomic>

class Client
{
protected:
	std::unique_ptr<std::thread> _worker; //Constructed in inherited runAsync()
	
	std::mutex _mutexChannels;

	std::atomic<bool> _isClientConnected;
	std::atomic<bool> _isClientRequestingData;

	virtual void tick() = 0;   //Used by the thread loop.

public:
	Client();
	virtual ~Client(); //If not virtual, if we delete the base class, the derived class will still exist.

	virtual void addToBuffers(uint8_t valueCH0, uint8_t valueCH1) = 0;
	
	bool isClientConnected();
	bool isClientRequestingData();

	virtual void runAsync(); //Used to start the thread.
};