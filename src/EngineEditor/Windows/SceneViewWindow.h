#pragma once

#include "WindowBase.h"
#include "Renderers/SceneViewRenderer.h"
#include "Utils/constants.h"

class SceneViewWindow : public WindowBase {

public:
	SceneViewWindow(const char* name, SceneViewRenderer* renderer);
	~SceneViewWindow();

protected:
	void OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) override;

private:
	SceneViewRenderer* m_Renderer;
};