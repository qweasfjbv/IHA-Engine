
#include "GameViewWindow.h"
#include "ImGUI/imgui.h"
#include "Utils/constants.h"

namespace IHA::Editor {

	GameViewWindow::GameViewWindow(const char* name, SceneViewRenderer* renderer)
		: WindowBase(name), m_Renderer(renderer) {}

	GameViewWindow::~GameViewWindow() {}


	void GameViewWindow::OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {

		ImVec2 size = ImGui::GetContentRegionAvail();
		UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
		srvHandle.ptr += IHA::SLOT_ID_GAMEVIEW * descriptorSize;

		// MSVC 호환 방식
		ImTextureID texId = (ImTextureID)(uintptr_t)(srvHandle.ptr);

		ImGui::Image(texId, size);
	}
}