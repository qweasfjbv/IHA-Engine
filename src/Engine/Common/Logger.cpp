#include "Logger.h"
#include <iostream>
#include <filesystem>
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

	void Logger::Log(const std::string& msg, LogLevel logLevel, const char* file, int line, const char* func) {

		std::string funcName(func);
		size_t pos = funcName.find_last_of("::");
		if (pos != std::string::npos)
			funcName = funcName.substr(pos + 1);

		std::cout << "[" <<  std::filesystem::path(file).filename().string() << ":" << line << " / " << funcName << "] "
			<< msg << std::endl;
	}

	void Logger::SetOutput(std::function<void(LogLevel, const std::string&)> callback)
	{
		// TODO - �����Ϳ��� �Լ� ���� ����
		// �����ص״ٰ� Log�Լ����� �ݹ� ȣ�� �ʿ�
	}

}