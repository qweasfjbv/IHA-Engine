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

namespace IHA::Editor {

    //struct ExampleDescriptorHeapAllocator
    //{
    //    ID3D12DescriptorHeap* Heap = nullptr;
    //    D3D12_DESCRIPTOR_HEAP_TYPE  HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
    //    D3D12_CPU_DESCRIPTOR_HANDLE HeapStartCpu;
    //    D3D12_GPU_DESCRIPTOR_HANDLE HeapStartGpu;
    //    UINT                        HeapHandleIncrement;
    //    ImVector<int>               FreeIndices;
    //    void Create(ID3D12Device* device, ID3D12DescriptorHeap* heap)
    //    {
    //        IM_ASSERT(Heap == nullptr && FreeIndices.empty());
    //        Heap = heap;
    //        D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
    //        HeapType = desc.Type;
    //        HeapStartCpu = Heap->GetCPUDescriptorHandleForHeapStart();
    //        HeapStartGpu = Heap->GetGPUDescriptorHandleForHeapStart();
    //        HeapHandleIncrement = device->GetDescriptorHandleIncrementSize(HeapType);
    //        FreeIndices.reserve((int)desc.NumDescriptors);
    //        for (int n = desc.NumDescriptors; n > 0; n--)
    //            FreeIndices.push_back(n - 1);
    //    }
    //    void Destroy()
    //    {
    //        Heap = nullptr;
    //        FreeIndices.clear();
    //    }
    //    void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
    //    {
    //        IM_ASSERT(FreeIndices.Size > 0);
    //        int idx = FreeIndices.back();
    //        FreeIndices.pop_back();
    //        out_cpu_desc_handle->ptr = HeapStartCpu.ptr + (idx * HeapHandleIncrement);
    //        out_gpu_desc_handle->ptr = HeapStartGpu.ptr + (idx * HeapHandleIncrement);
    //    }
    //    void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
    //    {
    //        int cpu_idx = (int)((out_cpu_desc_handle.ptr - HeapStartCpu.ptr) / HeapHandleIncrement);
    //        int gpu_idx = (int)((out_gpu_desc_handle.ptr - HeapStartGpu.ptr) / HeapHandleIncrement);
    //        IM_ASSERT(cpu_idx == gpu_idx);
    //        FreeIndices.push_back(cpu_idx);
    //    }
    //};

    class EngineEditor {

    public:
        bool Init(HWND hwnd);
        void MainLoop();
        void ShutDown();

        bool IsAbleToRenderFrame();

    private:
        void Update();
        void Render();

		bool InitImGUI(HWND hwnd);
        void InitDockedWindows();

        void DrawMainMenuBar();
        void DrawDockedWindows();

    private:

        HWND m_hWnd = nullptr;
        IHA::Engine::EngineCore* m_engineCore = nullptr;
        std::vector<std::unique_ptr<WindowBase>> m_Windows;

        static bool g_dockInit;
        static ImVec4 clear_color;
    };
}