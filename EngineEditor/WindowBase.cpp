
#include "WindowBase.h"
#include "../StaticThirdParty/ImGUI/imgui.h"

WindowBase::~WindowBase() {}

void WindowBase::Draw(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {

	ImGui::Begin(m_Name.c_str());

	// TODO - Check conditions (ex. visibility)
	OnGUI(device, srvHeap);
	ImGui::End();
}
