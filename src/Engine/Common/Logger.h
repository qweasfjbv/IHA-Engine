#pragma once

#include <string>
#include <functional>

#define LOG_INFO(msg)		Logger::Log(msg, LogLevel::Info, __FILE__, __LINE__, __FUNCTION__)
#define LOG_WARNING(msg)	Logger::Log(msg, LogLevel::Warning, __FILE__, __LINE__, __FUNCTION__)
#define LOG_ERROR(msg)		Logger::Log(msg, LogLevel::Error, __FILE__, __LINE__, __FUNCTION__)

namespace IHA::Engine {
	enum class LogLevel { Info, Warning, Error, System };

	static class Logger {

	public:
		static void Init();
		static void Shutdown();

		static void Log(const std::string& msg, LogLevel logLevel, const char* file, int line, const char* func);
		static void SetOutput(std::function<void(LogLevel, const std::string&)> callback);
	};
}
