#pragma once

#include "WindowBase.h"
#include "Renderers/SceneViewRenderer.h"
#include "Utils/constants.h"

namespace IHA::Editor {

	class GameViewWindow : public WindowBase {

	public:
		GameViewWindow(const char* name, SceneViewRenderer* renderer);
		~GameViewWindow();

	protected:
		void OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) override;

	private:
		// HACK - ���߿� GameViewRenderer�� ��ü �ʿ�
		SceneViewRenderer* m_Renderer;
	};
}