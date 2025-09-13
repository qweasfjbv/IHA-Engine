#pragma once

#include "WindowBase.h"
#include "SceneViewRenderer.h"
#include "constants.h"

class SceneViewWindow : public WindowBase {

public:
	SceneViewWindow(const char* name, SceneViewRenderer* renderer);
	~SceneViewWindow();

protected:
	void OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) override;

private:
	SceneViewRenderer* m_Renderer;
};