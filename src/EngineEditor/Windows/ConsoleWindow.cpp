
#include "ConsoleWindow.h"
#include "ImGUI/imgui.h"

namespace IHA::Editor {
	
	ConsoleWindow::ConsoleWindow(const char* name)
		: WindowBase(name) {}

	ConsoleWindow::~ConsoleWindow() {}

	void ConsoleWindow::OnGUI() {

		ImGui::Text(u8"�ܼ�â �ؽ�Ʈ");

	}
}