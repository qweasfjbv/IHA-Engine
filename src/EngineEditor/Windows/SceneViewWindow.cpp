
#include "SceneViewWindow.h"
#include "../StaticThirdParty/ImGUI/imgui.h"
#include "Utils/constants.h"

namespace IHA::Editor {

	SceneViewWindow::SceneViewWindow(const char* name, IHA::Engine::Renderer* renderer)
		: WindowBase(name), m_renderer(renderer) {}

	SceneViewWindow::~SceneViewWindow() {}

	void SceneViewWindow::OnGUI() {
		
		m_renderer->Resize(1280, 720);
		m_renderer->Render(nullptr, nullptr);

		ImVec2 size = ImGui::GetContentRegionAvail();
		ImTextureID texId = (ImTextureID)(uintptr_t)(m_renderer->GetSrvGpuDescHandle().ptr);

		ImGui::Image(texId, size);
	}
}