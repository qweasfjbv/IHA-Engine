#pragma once

#include "WindowBase.h"

namespace IHA::Editor {

	class ConsoleWindow : public WindowBase {

	public:
		ConsoleWindow(const char* name);
		~ConsoleWindow();

	protected:
		void OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) override;

		// private:

	};
}