#include <fstream>
#include "logger.h"

constexpr int LogBufferSize = 1024;

Logger::Logger(std::string logFile)
{
	_logBuffer = new char[LogBufferSize];
	_logFile = logFile;
}

Logger::~Logger()
{
	delete[] _logBuffer;
}

void Logger::Clear()
{
	std::ofstream fileStream(_logFile, std::ofstream::out);
	fileStream.close();
}

void Logger::Critical(const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	Log(format, arguments, "[Critical] ");
	va_end(arguments);
}

void Logger::Error(const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	Log(format, arguments, "[Error] ");
	va_end(arguments);
}

void Logger::Warning(const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	Log(format, arguments, "[Warning] ");
	va_end(arguments);
}

void Logger::Informational(const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	Log(format, arguments, "");
	va_end(arguments);
}

void Logger::Debug(const char* format, ...)
{
#ifdef _DEBUG
	va_list arguments;
	va_start(arguments, format);
	Log(format, arguments, "[Debug] ");
	va_end(arguments); 
#endif
}

void Logger::Trace(const char* format, ...)
{
#ifdef _TRACELOG
	va_list arguments;
	va_start(arguments, format);
	Log(format, arguments, "[Trace] ");
	va_end(arguments); 
#endif
}

void Logger::Log(const char* format, va_list arguments, const char* prefix)
{
	vsnprintf(_logBuffer, LogBufferSize, format, arguments);

	std::ofstream fileStream(_logFile, std::ofstream::out | std::ofstream::app);
	fileStream.write(prefix, strlen(prefix));
	fileStream.write(_logBuffer, strlen(_logBuffer));
	fileStream.close();
}