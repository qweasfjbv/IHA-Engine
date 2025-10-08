#pragma once

// Standard libs
#include <vector>
#include <memory>
#include <iostream>
#include "framework.h"

// Third party libs
#include "thirdparty.h"

// Project Modules
#include "EngineCore.h"
#include "Windows/WindowBase.h"

namespace IHA::Editor {

    class EngineEditor {

    public:
        bool Init(HWND hwnd);
        void MainLoop();
        void Resize(LPARAM lparam);
        void ShutDown();
        bool IsAbleToRenderFrame();

    private:
        void Update();
        void Render();

		bool InitImGUI(HWND hwnd);
        void InitDockedWindows();

        void DrawMainMenuBar();
        void DrawDockedWindows();

    public:
        inline static bool g_dockInit = false;
        static ImVec4 clear_color;

    private:

        HWND m_hWnd = nullptr;
        IHA::Engine::EngineCore* m_engineCore = nullptr;
        std::vector<std::unique_ptr<WindowBase>> m_windows;

    };
}