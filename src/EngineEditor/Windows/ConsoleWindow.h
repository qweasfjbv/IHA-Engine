#pragma once

#include "WindowBase.h"

namespace IHA::Editor {

	class ConsoleWindow : public WindowBase {

	public:
		ConsoleWindow(const char* name);
		~ConsoleWindow();

	protected:
		void OnGUI() override;

		// private:

	};
}