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
		// TODO - �����Ϳ��� �Լ� ���� ����
		// �����ص״ٰ� Log�Լ����� �ݹ� ȣ�� �ʿ�
	}

}