#pragma once

#include "WindowBase.h"

namespace IHA::Editor {

	class InspectorWindow : public WindowBase {

	public:
		InspectorWindow(const char* name);
		~InspectorWindow();

	protected:
		void OnGUI() override;

		// private:

	};
}