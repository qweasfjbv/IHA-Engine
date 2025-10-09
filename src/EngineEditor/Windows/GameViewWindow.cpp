
#include "GameViewWindow.h"
#include "ImGUI/imgui.h"
#include "Utils/constants.h"

namespace IHA::Editor {

	GameViewWindow::GameViewWindow(const char* name, IHA::Engine::Renderer* renderer)
		: WindowBase(name), m_renderer(renderer) {}

	GameViewWindow::~GameViewWindow() {}

	void GameViewWindow::OnGUI() {

		m_renderer->Resize(1280, 720);
		m_renderer->Render(nullptr, nullptr);

		ImVec2 size = ImGui::GetContentRegionAvail();
		ImTextureID texId = (ImTextureID)(uintptr_t)(m_renderer->GetSrvGpuDescHandle().ptr);

		ImGui::Image(texId, size);
	}
}