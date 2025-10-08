#pragma once

#include <string>
#include <functional>

namespace IHA::Engine {
	
	enum class LogLevel { Info, Warning, Error, System };

	static class Logger {

	public:
		static void Init();
		static void Shutdown();

		static void Log(const std::string message, LogLevel level = LogLevel::Info);
		static void SetOutput(std::function<void(LogLevel, const std::string&)> callback);
	};
}
