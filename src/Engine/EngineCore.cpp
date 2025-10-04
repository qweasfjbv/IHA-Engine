#include "EngineCore.h"
#include <memory>
#include <iostream>

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

namespace IHA::Engine {

	bool EngineCore::Init(HWND hWnd)
	{
		if (!CreateDeviceD3D(hWnd)) return false;
	}

	void EngineCore::PreUpdate()
	{
	}

	void EngineCore::BeginFrame() 
	{
		// Reset command list
		FrameContext* frameCtx = WaitForNextFrameResources();
		UINT backBufferIdx = m_swapChain->GetCurrentBackBufferIndex();
		frameCtx->commandAllocator->Reset();
		m_commandList->Reset(frameCtx->commandAllocator, nullptr);

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

	void EngineCore::EndFrame()
	{
		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		m_commandList->ResourceBarrier(1, &barrier);
		m_commandList->Close();

		m_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&m_commandList);
	}

	void EngineCore::Update()
	{
		// TODO - module updates
	}

	void EngineCore::PostUpdate()
	{
	}

	void EngineCore::Render()
	{
	}

	void EngineCore::Present() 
	{
		HRESULT hr = m_swapChain->Present(1, 0);
		
		UINT64 fenceValue = m_fenceLastSignaledValue + 1;
		m_commandQueue->Signal(m_fence, fenceValue);
		m_fenceLastSignaledValue = fenceValue;
		m_frameContext[m_frameIndex % APP_NUM_FRAMES_IN_FLIGHT].fenceValue = fenceValue;
	}

	void EngineCore::ShutDown()
	{
		CleanupDeviceD3D();
	}

	bool EngineCore::IsSwapChainOccluded() 
	{
		if (!m_swapChain) return false;
		return (m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED);
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
			desc.NumDescriptors = APP_NUM_BACK_BUFFERS;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			desc.NodeMask = 1;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvDescHeap)) != S_OK)
				return false;

			m_rtvDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
			for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
			{
				m_renderTargetDescs[i] = rtvHandle;
				rtvHandle.ptr += m_rtvDescSize;
			}
		}

		{	/* Create Game SRV Heap */
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			m_cbvSrvUavDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			desc.NumDescriptors = APP_SRV_HEAP_SIZE;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			if (m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_cbvSrvUavDescHeap)) != S_OK)
				return false;

			g_srvDescHeapAlloc = new DescriptorHeapAllocator(m_cbvSrvUavDescHeap, APP_SRV_HEAP_SIZE, m_cbvSrvUavDescSize);
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

		{	/* Create Render Target */
			for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
			{
				ID3D12Resource* backBuffer = nullptr;
				m_swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
				m_device->CreateRenderTargetView(backBuffer, nullptr, m_renderTargetDescs[i]);
				m_renderTargetResources[i] = backBuffer;
			}
		}

		return true;
	}

	void EngineCore::CleanupDeviceD3D()
	{
		WaitForLastSubmittedFrame();

		for (UINT i = 0; i < APP_NUM_BACK_BUFFERS; i++)
			if (m_renderTargetResources[i]) { m_renderTargetResources[i]->Release(); m_renderTargetResources[i] = nullptr; }

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