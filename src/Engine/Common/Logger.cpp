#include "Logger.h"
#include <iostream>
#include <windows.h>

namespace IHA::Engine {

	void Logger::Init()
	{
		AllocConsole();
		FILE* fp;
		freopen_s(&fp, "CONOUT$", "w", stdout);
	}
	void Logger::Shutdown()
	{
		FreeConsole();
	}

	void Logger::Log(const std::string message, LogLevel logLevel)
	{
		// HACK
		std::cout << message << '\n';
	}

	void Logger::SetOutput(std::function<void(LogLevel, const std::string&)> callback)
	{
		// TODO - 에디터에서 함수 들어올 예정
		// 저장해뒀다가 Log함수에서 콜백 호출 필요
	}

}