#pragma once

#include "WindowBase.h"

namespace IHA::Editor {

	class HierarchyWindow : public WindowBase {

	public:
		HierarchyWindow(const char* name);
		~HierarchyWindow();

	protected:
		void OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) override;

		// private:

	};
}