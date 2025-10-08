#pragma once

#include "WindowBase.h"
#include "Utils/constants.h"
#include "Core/Renderer.h"

namespace IHA::Editor {

	class SceneViewWindow : public WindowBase {

	public:
		SceneViewWindow(const char* name, IHA::Engine::Renderer* renderer);
		~SceneViewWindow();

	protected:
		void OnGUI() override;

	private:
		IHA::Engine::Renderer* m_renderer;
	};
}