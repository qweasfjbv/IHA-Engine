
#include "InspectorWindow.h"
#include "ImGUI/imgui.h"

namespace IHA::Editor {

	InspectorWindow::InspectorWindow(const char* name)
		: WindowBase(name) {}

	InspectorWindow::~InspectorWindow() {}

	void InspectorWindow::OnGUI() {

		ImGui::Text(u8"�ν�����â �ؽ�Ʈ");

	}
}