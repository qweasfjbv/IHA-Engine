#include "EngineCore.h"
#include <memory>
#include <iostream>
#include <cassert>
#include <filesystem>

#include "Core/World.h"
#include "Core/Renderer.h"
#include "Common/Logger.h"

#include "Components/MeshRenderer.h"

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

namespace IHA::Engine {
		
	bool EngineCore::Init(HWND hWnd)
	{
		Logger::Init();
		if (!CreateDeviceD3D(hWnd)) return false;
		CreateRenderTarget();

		/* vv TEST CODES vv */
		m_meshRendererSystem = new MeshRendererSystem();
		MeshRenderer meshRenderer;
		meshRenderer.material = new Material();

		std::filesystem::path base = std::filesystem::current_path();
		auto shaderPath = base.parent_path().parent_path() / "src/shaders/cube.hlsl";
		meshRenderer.material->shader = new Shader(shaderPath, shaderPath, m_device);
		meshRenderer.material->texture = new Texture();

		m_meshRendererSystem->Add(0, meshRenderer);
		
		m_cyclables.push_back(m_meshRendererSystem);
		/* ^^ TEST CODES ^^ */

		return true;
	}

	void EngineCore::Resize(LPARAM lParam)
	{
		WaitForLastSubmittedFrame();
		CleanupRenderTarget();
		HRESULT result = m_swapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
		assert(SUCCEEDED(result) && "Failed to resize swapchain.");
		CreateRenderTarget();
	}

	void EngineCore::PreUpdate()
	{
	}

	void EngineCore::ResetCommands()
	{
		// Reset command list
		m_swapChainOccluded = false;
		FrameContext* frameCtx = WaitForNextFrameResources();
		frameCtx->commandAllocator->Reset();
		m_commandList->Reset(frameCtx->commandAllocator, nullptr);
	}

	void EngineCore::OpenBarrier()
	{
		UINT backBufferIdx = m_swapChain->GetCurrentBackBufferIndex();
		// Ready resource barrier
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_renderTargetResources[backBufferIdx];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		m_commandList->ResourceBarrier(1, &barrier);
	}

	void EngineCore::CloseBarrier()
	{
		UINT backBufferIdx = m_swapChain->GetCurrentBackBufferIndex();
		D3D12_RESOURCE_BARRIER barrier = {};		
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_renderTargetResources[backBufferIdx];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		m_commandList->ResourceBarrier(1, &barrier);
	}

	void EngineCore::Update()
	{
		// HACK - Timer 필요

		for (const auto& cycle : m_cyclables) {
			cycle->Update(0.01f);
		}
	}

	void EngineCore::PostUpdate()
	{
	}

	void EngineCore::Render()
	{

	}

	void EngineCore::Present() 
	{
		m_commandList->Close();
		m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&m_commandList);

		HRESULT hr = m_swapChain->Present(1, 0);
		m_swapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

		UINT64 fenceValue = m_fenceLastSignaledValue + 1;
		m_commandQueue->Signal(m_fence, fenceValue);
		m_fenceLastSignaledValue = fenceValue;
		FrameContext* frameCtx = &m_frameContext[m_frameIndex % APP_NUM_FRAMES_IN_FLIGHT];
		frameCtx->fenceValue = fenceValue;
	}

	void EngineCore::ShutDown()
	{
		CleanupRenderTarget();
		CleanupDeviceD3D();
	}

	bool EngineCore::IsSwapChainOccluded() 
	{
		if (!m_swapChain) return false;
		return m_swapChainOccluded;
	}

	Renderer* EngineCore::CreateRenderer(UINT width, UINT height)
	{
		DescHandles descHandles{};

		// TODO - handling error cause of lack of heaps' size
		g_rtvDescHeapAlloc->Alloc(&descHandles.m_rtvCPUDescHandle, &descHandles.m_rtvGPUDescHandle);
		g_srvDescHeapAlloc->Alloc(&descHandles.m_srvCPUDescHandle, &descHandles.m_srvGPUDescHandle);
		g_dsvDescHeapAlloc->Alloc(&descHandles.m_dsvCPUDescHandle, &descHandles.m_dsvGPUDescHandle);

		Renderer* renderer = new Renderer(m_device, m_commandList, std::move(descHandles));
		renderer->Resize(width, height);
		m_renderers.push_back(renderer);
		
		return renderer;
	}

	bool EngineCore::CreateDeviceD3D(HWND hwnd)
	{
		DXGI_SWAP_CHAIN_DESC1 sd; {
			ZeroMemory(&sd, sizeof(sd));
			sd.BufferCount = APP_NUM_BACK_BUFFERS;
			sd.Width = 0;
			sd.Height = 0;
			sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
			sd.Scaling = DXGI_SCALING_STRETCH;
			sd.Stereo = FALSE;
		}

#ifdef DX12_ENABLE_DEBUG_LAYER
		ID3D12Debug* dx12Debug = nullptr;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dx12Debug))))
			dx12Debug->EnableDebugLayer();
#endif

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
		if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&m_device)) != S_OK)
			return false;

#ifdef DX12_ENABLE_DEBUG_LAYER
		if (pdx12Debug != nullptr)
		{
			ID3D12InfoQueue* pInfoQueue = nullptr;
			g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			pInfoQueue->Release();
			pdx12Debug->Release();
		}
#endif

		{	/* Create RTV Heap */
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = RTV_HEAP_SIZE;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 1;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvDescHeap)) != S_OK)
				return false;

			m_rtvDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			g_rtvDescHeapAlloc = new DescriptorHeapAllocator(m_rtvDescHeap, RTV_HEAP_SIZE, m_rtvDescSize);

			D3D12_CPU_DESCRIPTOR_HANDLE rtvCpuHandle = {};
			D3D12_GPU_DESCRIPTOR_HANDLE rtvGpuHandle = {};
			for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
			{
				g_rtvDescHeapAlloc->Alloc(&rtvCpuHandle, &rtvGpuHandle);
				m_renderTargetDescs[i] = rtvCpuHandle;
			}
		}

		{	/* Create Game SRV Heap */
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			m_cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = SRV_HEAP_SIZE;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_cbvSrvUavDescHeap)) != S_OK)
				return false;
		}

		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			m_dsvDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			desc.NumDescriptors = DSV_HEAP_SIZE;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_dsvDescHeap)) != S_OK)
				return false;
		}

		{
			g_srvDescHeapAlloc = new DescriptorHeapAllocator(m_cbvSrvUavDescHeap, SRV_HEAP_SIZE, m_cbvSrvUavDescSize);
			g_dsvDescHeapAlloc = new DescriptorHeapAllocator(m_dsvDescHeap, DSV_HEAP_SIZE, m_dsvDescSize);
		}

		{	/* Create Command Objects */
			D3D12_COMMAND_QUEUE_DESC desc = {};
			desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
			desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			desc.NodeMask = 1;
			if (m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_commandQueue)) != S_OK)
				return false;

			for (UINT i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
				if (m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_frameContext[i].commandAllocator)) != S_OK)
					return false;

			if (m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_frameContext[0].commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList)) != S_OK ||
				m_commandList->Close() != S_OK)
				return false;
		}

		{	/* Create Fence */
			if (m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)) != S_OK)
				return false;

			m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (m_fenceEvent == nullptr)
				return false;
		}

		{	/* Create Swap Chain */
			IDXGIFactory4* dxgiFactory = nullptr;
			IDXGISwapChain1* swapChain1 = nullptr;
			if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
				return false;
			if (dxgiFactory->CreateSwapChainForHwnd(m_commandQueue, hwnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
				return false;
			if (swapChain1->QueryInterface(IID_PPV_ARGS(&m_swapChain)) != S_OK)
				return false;

			swapChain1->Release();
			dxgiFactory->Release();
			m_swapChain->SetMaximumFrameLatency(APP_NUM_BACK_BUFFERS);
			m_swapChainWaitableObject = m_swapChain->GetFrameLatencyWaitableObject();
		}

		return true;
	}

	void EngineCore::CleanupDeviceD3D()
	{
		WaitForLastSubmittedFrame();

		if (m_swapChain) { m_swapChain->SetFullscreenState(false, nullptr); m_swapChain->Release(); m_swapChain = nullptr; }
		if (m_swapChainWaitableObject != nullptr) { CloseHandle(m_swapChainWaitableObject); }
		for (UINT i = 0; i < APP_NUM_FRAMES_IN_FLIGHT; i++)
			if (m_frameContext[i].commandAllocator) { m_frameContext[i].commandAllocator->Release(); m_frameContext[i].commandAllocator = nullptr; }
		if (m_commandQueue) { m_commandQueue->Release(); m_commandQueue = nullptr; }
		if (m_commandList) { m_commandList->Release(); m_commandList = nullptr; }
		if (m_rtvDescHeap) { m_rtvDescHeap->Release(); m_rtvDescHeap = nullptr; }
		if (m_cbvSrvUavDescHeap) { m_cbvSrvUavDescHeap->Release(); m_cbvSrvUavDescHeap = nullptr; }
		if (m_fence) { m_fence->Release(); m_fence = nullptr; }
		if (m_fenceEvent) { CloseHandle(m_fenceEvent); m_fenceEvent = nullptr; }
		if (m_device) { m_device->Release(); m_device = nullptr; }

#ifdef DX12_ENABLE_DEBUG_LAYER
		IDXGIDebug1* pDebug = nullptr;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
		{
			pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
			pDebug->Release();
		}
#endif
	}

	void EngineCore::CreateRenderTarget()
	{
		for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
		{
			ID3D12Resource* backBuffer = nullptr;
			m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
			m_device->CreateRenderTargetView(backBuffer, nullptr, m_renderTargetDescs[i]);
			m_renderTargetResources[i] = backBuffer;
		}
	}

	void EngineCore::CleanupRenderTarget()
	{
		for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
			if (m_renderTargetResources[i]) { m_renderTargetResources[i]->Release(); m_renderTargetResources[i] = nullptr; }
	}

	void EngineCore::WaitForLastSubmittedFrame()
	{
		FrameContext* frameCtx = &m_frameContext[m_frameIndex % APP_NUM_FRAMES_IN_FLIGHT];

		UINT64 fenceValue = frameCtx->fenceValue;
		if (fenceValue == 0)
			return; // No fence was signaled

		frameCtx->fenceValue = 0;
		if (m_fence->GetCompletedValue() >= fenceValue)
			return;

		m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	FrameContext* EngineCore::WaitForNextFrameResources()
	{
		UINT nextFrameIndex = m_frameIndex + 1;
		m_frameIndex = nextFrameIndex;

		HANDLE waitableObjects[] = { m_swapChainWaitableObject, nullptr };
		DWORD numWaitableObjects = 1;

		FrameContext* frameCtx = &m_frameContext[nextFrameIndex % APP_NUM_FRAMES_IN_FLIGHT];
		UINT64 fenceValue = frameCtx->fenceValue;
		if (fenceValue != 0) // means no fence was signaled
		{
			frameCtx->fenceValue = 0;
			m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
			waitableObjects[1] = m_fenceEvent;
			numWaitableObjects = 2;
		}

		WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);
		return frameCtx;
	}
}