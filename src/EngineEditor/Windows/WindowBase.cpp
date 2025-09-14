
#include "WindowBase.h"
#include "../StaticThirdParty/ImGUI/imgui.h"

namespace IHA::Editor {

	WindowBase::~WindowBase() {}

	void WindowBase::Draw(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin(m_Name.c_str());
		ImGui::PopStyleVar(3);

		// TODO - Check conditions (ex. visibility)
		OnGUI(device, srvHeap);
		ImGui::End();
	}
}