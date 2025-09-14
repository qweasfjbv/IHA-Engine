
#include "InspectorWindow.h"
#include "ImGUI/imgui.h"

namespace IHA::Editor {

	InspectorWindow::InspectorWindow(const char* name)
		: WindowBase(name) {}

	InspectorWindow::~InspectorWindow() {}

	void InspectorWindow::OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {

		ImGui::Text(u8"인스펙터창 텍스트");

	}
}