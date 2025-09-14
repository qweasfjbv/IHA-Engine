
#include "ConsoleWindow.h"
#include "ImGUI/imgui.h"

namespace IHA::Editor {
	
	ConsoleWindow::ConsoleWindow(const char* name)
		: WindowBase(name) {}

	ConsoleWindow::~ConsoleWindow() {}

	void ConsoleWindow::OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {

		ImGui::Text(u8"콘솔창 텍스트");

	}
}