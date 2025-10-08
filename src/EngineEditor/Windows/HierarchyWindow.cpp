
#include "HierarchyWindow.h"
#include "ImGUI/imgui.h"

namespace IHA::Editor {

	HierarchyWindow::HierarchyWindow(const char* name)
		: WindowBase(name) {}

	HierarchyWindow::~HierarchyWindow() {}

	void HierarchyWindow::OnGUI() {

		ImGui::Text(u8"계층창 텍스트");

	}
}