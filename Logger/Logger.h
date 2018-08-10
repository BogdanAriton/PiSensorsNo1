#pragma once
#include <string>
#include <mutex>
#include <memory>
#include "Configuration.h"

class Logger
{
public:
	enum LOG_LEVEL {
		Trace,
		Info,
		Debug,
		Warning,
		Error
	};

	static Logger * getInstance();

	void setLevel(int logLevel);
	void logTrace(std::string message);
	void logInfo(std::string message);
	void logDebug(std::string message);
	void logWarning(std::string message);
	void logError(std::string message);

private:
	Logger();
	Logger(Logger const&) {};
	void operator=(Logger const&) {};

	LOG_LEVEL _logLevel;
	std::mutex _logMutex;

	//One file per day; filepath format : LOG_[year]_[month]_[day]
	std::string _logFilePath;
	int _todayFiles;

	void setLogFilePathAndNumberOfFiles(std::string filePathTemplate);
	bool fileExists(std::string fileName);
	long getFileSize();
	void appendToFile(std::string message);

	std::string getDate();
	std::string getTime();

	std::shared_ptr<Configuration::CFG_LOGGER> _config;
};
