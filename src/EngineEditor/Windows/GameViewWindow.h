#pragma once

#include "WindowBase.h"
#include "Utils/constants.h"
#include "Core/Renderer.h"

namespace IHA::Editor {

	class GameViewWindow : public WindowBase {

	public:
		GameViewWindow(const char* name, IHA::Engine::Renderer* renderer);
		~GameViewWindow();

	protected:
		void OnGUI() override;

	private:
		IHA::Engine::Renderer* m_renderer;
	};
}