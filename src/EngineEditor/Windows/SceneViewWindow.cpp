
#include "SceneViewWindow.h"
#include "../StaticThirdParty/ImGUI/imgui.h"
#include <iostream>

SceneViewWindow::SceneViewWindow(const char* name, SceneViewRenderer* renderer)
	: WindowBase(name), m_Renderer(renderer) {}

SceneViewWindow::~SceneViewWindow() {}


void SceneViewWindow::OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) {

	ImVec2 size = ImGui::GetContentRegionAvail();
	UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12_GPU_DESCRIPTOR_HANDLE srvHandle = srvHeap->GetGPUDescriptorHandleForHeapStart();
	srvHandle.ptr += 2 * descriptorSize;

	// MSVC 호환 방식
	ImTextureID texId = (ImTextureID)(uintptr_t)(srvHandle.ptr);

	ImGui::Image(texId, size);
}