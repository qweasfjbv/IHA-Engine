#pragma once

#include "WindowBase.h"

namespace IHA::Editor {

	class HierarchyWindow : public WindowBase {

	public:
		HierarchyWindow(const char* name);
		~HierarchyWindow();

	protected:
		void OnGUI() override;

		// private:

	};
}