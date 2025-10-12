#include "EngineEditor.h"

#include "Core/Renderer.h"
#include "Common/Logger.h"

// Windows
#include "Windows/SceneViewWindow.h"
#include "Windows/GameViewWindow.h"
#include "Windows/HierarchyWindow.h"
#include "Windows/InspectorWindow.h"
#include "Windows/ConsoleWindow.h"

// Utilties
#include "Utils/constants.h"
#include "Utils/Enums.h"


using namespace IHA::Engine;

namespace IHA::Editor {

    ImVec4 EngineEditor::clear_color{};

    bool EngineEditor::Init(HWND hWnd)
    {
        m_engineCore = new EngineCore();
        if (!m_engineCore->Init(hWnd)) return false;
        if (!InitImGUI(hWnd)) return false;

        m_hWnd = hWnd;
        InitDockedWindows();

        LOG_INFO("Init Success");

        return true;
    }

    void EngineEditor::MainLoop()
    {
        m_engineCore->ResetCommands();

        m_engineCore->PreUpdate();
        m_engineCore->Update();
        Update();
        m_engineCore->PostUpdate();

        ImGui::Render();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        m_engineCore->OpenBarrier();
        Render();
        m_engineCore->CloseBarrier();
        
        m_engineCore->Present();
    }

    void EngineEditor::Resize(LPARAM lParam)
    {
        m_engineCore->Resize(lParam);
    }

    void EngineEditor::Update()
    {
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        DrawMainMenuBar();
        DrawDockedWindows();
    }

    void EngineEditor::Render()
    {
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        UINT backBufferIdx = m_engineCore->GetSwapChain()->GetCurrentBackBufferIndex();
        auto rtvHandle = m_engineCore->GetMainRTVHandle(backBufferIdx);
        auto cmd = m_engineCore->GetCommandList();

        cmd->ClearRenderTargetView(rtvHandle, clear_color_with_alpha, 0, nullptr);
        cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        ID3D12DescriptorHeap* heaps[] = { m_engineCore->GetSrvDescHeap() };
        cmd->SetDescriptorHeaps(1, heaps);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_engineCore->GetCommandList());
    }

    void EngineEditor::ShutDown()
    {
        if (m_engineCore != nullptr) { 
            m_engineCore->WaitForLastSubmittedFrame();
            m_engineCore->ShutDown();
            m_engineCore = nullptr; 
        }
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        m_hWnd = nullptr;
    }

    bool EngineEditor::IsAbleToRenderFrame()
    {
        if (m_engineCore == nullptr || m_hWnd == nullptr) return false;
        if (IsIconic(m_hWnd)) return false;
        if (m_engineCore->IsSwapChainOccluded()) return false;

        return true;
    }

    bool EngineEditor::InitImGUI(HWND hwnd)
    {
        ::ShowWindow(hwnd, SW_SHOWDEFAULT);
        ::UpdateWindow(hwnd);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        UINT main_scale = GetDpiForWindow(hwnd) / 96.0f;    // NOTE: Devide by default DPI
        style.ScaleAllSizes(main_scale);
        style.FontScaleDpi = main_scale;

        if (!ImGui_ImplWin32_Init(hwnd)) return false;

        ImGui_ImplDX12_InitInfo init_info = {};
        init_info.Device = m_engineCore->GetDevice();
        init_info.CommandQueue = m_engineCore->GetCommandQueue();
        init_info.NumFramesInFlight = EngineCore::APP_NUM_FRAMES_IN_FLIGHT;
        init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

        init_info.SrvDescriptorHeap = m_engineCore->GetSrvDescHeap();
        init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu) 
            { return EngineCore::g_srvDescHeapAlloc->Alloc(outCpu, outGpu); };
        init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu) 
            { return EngineCore::g_srvDescHeapAlloc->Free(cpu, gpu); };
        ImGui_ImplDX12_Init(&init_info);

        std::string fontPath = "../../" + std::string(IHA::PATH_FONT) + std::string(IHA::FONT_NAME);
        ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f, nullptr, io.Fonts->GetGlyphRangesKorean());
        io.Fonts->Build();
        io.FontDefault = font;

        return true;
    }

    void EngineEditor::InitDockedWindows()
    {
        Renderer* sceneRenderer = m_engineCore->CreateRenderer(1280, 720);
        Renderer* gameRenderer = m_engineCore->CreateRenderer(1280, 720);
        m_windows.push_back(std::make_unique<SceneViewWindow>(IHA::WINDOW_NAME_SCENEVIEW, sceneRenderer));
        m_windows.push_back(std::make_unique<GameViewWindow>(IHA::WINDOW_NAME_GAMEVIEW, gameRenderer));
        m_windows.push_back(std::make_unique<ConsoleWindow>(IHA::WINDOW_NAME_CONSOLE));
        m_windows.push_back(std::make_unique<HierarchyWindow>(IHA::WINDOW_NAME_HIERARCHY));
        m_windows.push_back(std::make_unique<InspectorWindow>(IHA::WINDOW_NAME_INSPECTOR));
    }

    void EngineEditor::DrawMainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu(u8"파일"))
            {
                if (ImGui::MenuItem(u8"새로 만들기")) {}
                if (ImGui::MenuItem(u8"열기")) {}
                if (ImGui::MenuItem(u8"저장")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu(u8"설정"))
            {
                if (ImGui::MenuItem(u8"환경설정")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void EngineEditor::DrawDockedWindows()
    {
        ImGuiWindowFlags fullscreen_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoNavFocus;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        float menuBarHeight = ImGui::GetFrameHeight();

        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin(IHA::WINDOW_NAME_FULLSCREEN, nullptr, fullscreen_flags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID(IHA::DOCK_NAME_DEFAULT);
        ImGui::DockSpace(dockspace_id, ImVec2(.0f, .0f), ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();

        // OnGUI
        for (auto& window : m_windows) {
            window->Draw();
        }

        if (!g_dockInit) {
            g_dockInit = true;

            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);
            ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

            ImGuiID dock_left, dock_center, dock_right;
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.6f, &dock_left, &dock_center);
            ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Right, 0.5f, &dock_right, &dock_center);

            ImGuiID dock_left_top, dock_left_bottom;
            ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.5f, &dock_left_top, &dock_left_bottom);

            ImGuiID dock_center_top, dock_center_bottom;
            ImGui::DockBuilderSplitNode(dock_center, ImGuiDir_Up, 0.5f, &dock_center_top, &dock_center_bottom);

            ImGuiID dock_right_panel = dock_right;

            ImGui::DockBuilderDockWindow(IHA::WINDOW_NAME_SCENEVIEW, dock_left_top);
            ImGui::DockBuilderDockWindow(IHA::WINDOW_NAME_GAMEVIEW, dock_left_bottom);
            ImGui::DockBuilderDockWindow(IHA::WINDOW_NAME_HIERARCHY, dock_center_top);
            ImGui::DockBuilderDockWindow(IHA::WINDOW_NAME_CONSOLE, dock_center_bottom);
            ImGui::DockBuilderDockWindow(IHA::WINDOW_NAME_INSPECTOR, dock_right_panel);

            ImGui::DockBuilderFinish(dockspace_id);
        }

    }

}