#pragma once
#include <string>
#include <cstdarg>

class Logger {
public:
	Logger(std::string logFile);
	virtual ~Logger();

	void Clear();

	void Critical(const char* format, ...);
	void Error(const char* format, ...);
	void Warning(const char* format, ...);
	void Informational(const char* format, ...);
	void Debug(const char* format, ...);
	void Trace(const char* format, ...);

protected:
	char* _logBuffer;
	std::string _logFile;
	void Log(const char* format, va_list arguments, const char* prefix);
};
