#include <iostream>
#include <fstream>
#include <memory>
#include "Logger.h"

#define MB_BYTES_SIZE 1024 * 1024

using namespace std;

Logger::Logger()
{
	_config = Configuration::getInstance()->getConfiguration()->Logger;
	//Building log file
	//Set current log filepath
	std::string filePathTemplate = "LOG_" + getDate() + "_";
	setLogFilePathAndNumberOfFiles(filePathTemplate);
	//Set log level from config
	_logLevel = static_cast<LOG_LEVEL>(_config->_logLevel);
}

Logger * Logger::getInstance()
{
	static Logger logger;
	return &logger;
}

void Logger::setLevel(int logLevel)
{
	switch (logLevel)
	{
	case 0:
	{
		_logLevel = LOG_LEVEL::Trace;
		break;
	}
	case 1:
	{
		_logLevel = LOG_LEVEL::Info;
		break;
	}
	case 2:
	{
		_logLevel = LOG_LEVEL::Debug;
		break;
	}
	case 3:
	{
		_logLevel = LOG_LEVEL::Warning;
		break;
	}
	case 4:
	{
		_logLevel = LOG_LEVEL::Error;
		break;
	}
	default:
	{
		_logLevel = static_cast<LOG_LEVEL>(_config->_logLevel);
		break;
	}

	}
}

bool Logger::fileExists(std::string fileName)
{
	std::ifstream ifile(fileName);
	if (ifile)
	{
		ifile.close();
		return true;
	}
	ifile.close();
	return false;
}

void Logger::setLogFilePathAndNumberOfFiles(std::string filePathTemplate)
{
	int it = 1;
	bool noFile = false;
	std::string filePath;
	std::string filePathToSearch = filePathTemplate + std::to_string(it) + "." + _config->_fileExtension;

	while (noFile == false) {
		noFile = true;
		if (fileExists(filePathToSearch))
		{
			filePath = filePathToSearch;
			noFile = false;
			it++;
			filePathToSearch = filePathTemplate + std::to_string(it) + "." + _config->_fileExtension;
		}
	}

	if (it == 1) {
		_logFilePath = filePathToSearch;
		_todayFiles = it;
	}
	else {
		_logFilePath = filePath;
		_todayFiles = it - 1;
	}
}

long Logger::getFileSize()
{
	std::streampos fsize = 0;

	std::ifstream myfile(_logFilePath, ios::in);  // File is of type const char*

	fsize = myfile.tellg();         // The file pointer is currently at the beginning
	myfile.seekg(0, ios::end);      // Place the file pointer at the end of file

	fsize = myfile.tellg() - fsize;
	myfile.close();

	static_assert(sizeof(fsize) >= sizeof(long), "Oops.");

	return fsize;
}

void Logger::appendToFile(std::string message)
{
	message = message + "\n";

	if (getFileSize() + message.size() + 2 > _config->_maxFileSize * MB_BYTES_SIZE) {
		_todayFiles++;
		_logFilePath = "LOG_" + getDate() + "_" + std::to_string(_todayFiles) + "." + _config->_fileExtension;
	}

	std::ofstream outFile(_logFilePath, ios::app);

	if (outFile.is_open())
	{
		//If current file path file + data to be written is too large, create a new file
		if (!(outFile << message))
		{
			std::cerr << "Failed writing to file.";
		}
		outFile.close();
	}
	else
	{
		std::cerr << "Failed opening file.";
	}
}

std::string Logger::getDate()
{
	time_t now = time(0);
	tm * timeInfo = localtime(&now);

	std::string year = std::to_string(timeInfo->tm_year + 1900);
	std::string month = (((timeInfo->tm_mon + 1) / 10 == 0) ? ("0" + std::to_string(timeInfo->tm_mon + 1)) : std::to_string(timeInfo->tm_mon + 1));
	std::string day = (((timeInfo->tm_mday) / 10 == 0) ? ("0" + std::to_string(timeInfo->tm_mday)) : std::to_string(timeInfo->tm_mday));

	return year + "_" + month + "_" + day;
}

std::string Logger::getTime()
{
	time_t now = time(0);
	tm * timeInfo = localtime(&now);

	std::string hour = (((timeInfo->tm_hour) / 10 == 0) ? ("0" + std::to_string(timeInfo->tm_hour)) : std::to_string(timeInfo->tm_hour));
	std::string min = (((timeInfo->tm_min) / 10 == 0) ? ("0" + std::to_string(timeInfo->tm_min)) : std::to_string(timeInfo->tm_min));
	std::string sec = (((timeInfo->tm_sec) / 10 == 0) ? ("0" + std::to_string(timeInfo->tm_sec)) : std::to_string(timeInfo->tm_sec));

	return hour + ":" + min + ":" + sec;
}

void Logger::logTrace(std::string message)
{
	if (_logLevel < 1)
	{
		_logMutex.lock();
		std::string message_detail = "[Log TRACE - " + getTime() + "]" + "\n\t" + message;

		if (_config->_logToConsole)
			std::cout << message_detail << endl;
		if (_config->_logToFile)
			appendToFile(message_detail);
		if (_config->_logToDatabase)
			//log to database when database connection will work on c++ 20 years later
			cout << endl;
		_logMutex.unlock();
	}
}

void Logger::logInfo(std::string message)
{
	if (_logLevel < 2)
	{
		_logMutex.lock();
		std::string message_detail = "[Log INFO - " + getTime() + "]" + "\n\t" + message;

		if (_config->_logToConsole)
			std::cout << message_detail << endl;
		if (_config->_logToFile)
			appendToFile(message_detail);
		if (_config->_logToDatabase)
			//log to database when database connection will work on c++ 20 years later
			cout << endl;
		_logMutex.unlock();
	}
}

void Logger::logDebug(std::string message)
{
	if (_logLevel < 3)
	{
		_logMutex.lock();
		std::string message_detail = "[Log DEBUG - " + getTime() + "]" + "\n\t" + message;

		if (_config->_logToConsole)
			std::cout << message_detail << endl;
		if (_config->_logToFile)
			appendToFile(message_detail);
		if (_config->_logToDatabase)
			//log to database when database connection will work on c++ 20 years later
			cout << endl;
		_logMutex.unlock();
	}
}

void Logger::logWarning(std::string message)
{
	if (_logLevel < 4)
	{
		_logMutex.lock();
		std::string message_detail = "[Log WARNING - " + getTime() + "]" + "\n\t" + message;

		if (_config->_logToConsole)
			std::cout << message_detail << endl;
		if (_config->_logToFile)
			appendToFile(message_detail);
		if (_config->_logToDatabase)
			//log to database when database connection will work on c++ 20 years later
			cout << endl;
		_logMutex.unlock();
	}
}

void Logger::logError(std::string message)
{
	if (_logLevel < 5)
	{
		_logMutex.lock();
		std::string message_detail = "[Log ERROR - " + getTime() + "]" + "\n\t" + message;

		if (_config->_logToConsole)
			std::cout << message_detail << endl;
		if (_config->_logToFile)
			appendToFile(message_detail);
		if (_config->_logToDatabase)
			//log to database when database connection will work on c++ 20 years later
			cout << endl;
		_logMutex.unlock();
	}
}