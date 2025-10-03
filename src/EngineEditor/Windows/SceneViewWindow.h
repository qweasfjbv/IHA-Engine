#pragma once

#include "WindowBase.h"
#include "Renderers/Renderer.h"
#include "Utils/constants.h"

namespace IHA::Editor {

	class SceneViewWindow : public WindowBase {

	public:
		SceneViewWindow(const char* name, Renderer* renderer);
		~SceneViewWindow();

	protected:
		void OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) override;

	private:
		Renderer* m_Renderer;
	};
}