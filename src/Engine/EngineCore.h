#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Common/Logger.h>


namespace IHA::Engine {

	/* Forward Declarations */
	class Renderer;
	class SceneGraph;
	class ResourceManager;
	class World;
	struct ICyclable;

	struct FrameContext
	{
		ID3D12CommandAllocator*		commandAllocator;
		UINT64                      fenceValue;
	};

	class DescriptorHeapAllocator
	{
	public:
		DescriptorHeapAllocator(ID3D12DescriptorHeap* heap, UINT heapSize, UINT incrementSize)
			: m_heap(heap), m_heapHandleIncrement(incrementSize), m_heapSize(heapSize)
		{
			assert(m_heap != nullptr);
			D3D12_DESCRIPTOR_HEAP_DESC desc = heap->GetDesc();
			m_heapStartCpu = m_heap->GetCPUDescriptorHandleForHeapStart();
			m_heapStartGpu = m_heap->GetGPUDescriptorHandleForHeapStart();
			m_freeIndices.reserve((int)heapSize);
			for (int n = heapSize; n > 0; n--)
				m_freeIndices.push_back(n - 1);
		}
		~DescriptorHeapAllocator()
		{
			m_heap = nullptr;
			m_freeIndices.clear();
		}
		void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle)
		{
			assert(m_freeIndices.size() > 0);
			int idx = m_freeIndices.back();
			m_freeIndices.pop_back();
			out_cpu_desc_handle->ptr = m_heapStartCpu.ptr + (idx * m_heapHandleIncrement);
			out_gpu_desc_handle->ptr = m_heapStartGpu.ptr + (idx * m_heapHandleIncrement);
		}
		void Free(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle)
		{
			int cpu_idx = (int)((out_cpu_desc_handle.ptr - m_heapStartCpu.ptr) / m_heapHandleIncrement);
			int gpu_idx = (int)((out_gpu_desc_handle.ptr - m_heapStartGpu.ptr) / m_heapHandleIncrement);

			m_freeIndices.push_back(cpu_idx);
		}

	private:
		ID3D12DescriptorHeap*		m_heap = nullptr;
		D3D12_CPU_DESCRIPTOR_HANDLE m_heapStartCpu;
		D3D12_GPU_DESCRIPTOR_HANDLE m_heapStartGpu;
		UINT                        m_heapHandleIncrement;
		UINT						m_heapSize;
		std::vector<int>            m_freeIndices;
	};

	class EngineCore {

	public:
		bool Init(HWND hwnd);
		void Resize(LPARAM lParam);
		void ShutDown();

		void ResetCommands();
		void OpenBarrier();
		void CloseBarrier();

		void PreUpdate();
		void Update();
		void PostUpdate();
		void Render();
		void Present();

		void WaitForLastSubmittedFrame();
		bool IsSwapChainOccluded();

		Renderer* CreateRenderer(UINT width, UINT height);

	private:
		bool CreateDeviceD3D(HWND hwnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
		FrameContext* WaitForNextFrameResources();

	public:
		FORCEINLINE ID3D12Device* GetDevice() const { return m_device; }
		FORCEINLINE IDXGISwapChain3* GetSwapChain() const { return m_swapChain; }
		FORCEINLINE ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue; }
		FORCEINLINE ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList; }
		FORCEINLINE ID3D12DescriptorHeap* GetSrvDescHeap() const { return m_cbvSrvUavDescHeap; }

		FORCEINLINE D3D12_CPU_DESCRIPTOR_HANDLE GetMainRTVHandle(int backBufferIndex) const {
			return m_renderTargetDescs[backBufferIndex];
		}

		FORCEINLINE SceneGraph* GetScene() const { return m_sceneGraph; }
		FORCEINLINE ResourceManager* GetResourceManager() const { return m_resourceManager; }

		static const int APP_NUM_FRAMES_IN_FLIGHT = 2;
		static const int APP_NUM_BACK_BUFFERS = 2;

		static const int SRV_HEAP_SIZE = 16;
		static const int RTV_HEAP_SIZE = 16;
		static const int DSV_HEAP_SIZE = 16;

		inline static DescriptorHeapAllocator* g_srvDescHeapAlloc = nullptr;
		inline static DescriptorHeapAllocator* g_rtvDescHeapAlloc = nullptr;
		inline static DescriptorHeapAllocator* g_dsvDescHeapAlloc = nullptr;

	private:

		/* D3D12 Graphics */
		ID3D12Device*					m_device = nullptr;
		IDXGISwapChain3*				m_swapChain = nullptr;
		HANDLE							m_swapChainWaitableObject = nullptr;
		ID3D12CommandQueue*				m_commandQueue = nullptr;
		ID3D12GraphicsCommandList*		m_commandList = nullptr;
		FrameContext					m_frameContext[APP_NUM_FRAMES_IN_FLIGHT] = {};
		UINT							m_frameIndex = 0;
		UINT64                          m_fenceLastSignaledValue = 0;

		ID3D12DescriptorHeap*			m_rtvDescHeap = nullptr;
		ID3D12DescriptorHeap*			m_dsvDescHeap = nullptr;
		ID3D12DescriptorHeap*			m_cbvSrvUavDescHeap = nullptr;
		UINT							m_rtvDescSize;
		UINT							m_dsvDescSize;
		UINT							m_cbvSrvUavDescSize;

		ID3D12Resource*					m_renderTargetResources[APP_NUM_BACK_BUFFERS] = {};
		D3D12_CPU_DESCRIPTOR_HANDLE		m_renderTargetDescs[APP_NUM_BACK_BUFFERS] = {};
		ID3D12Resource*					m_depthStencil;
		DXGI_FORMAT						m_backBufferFormat;
		DXGI_FORMAT						m_depthStencilFormat;

		ID3D12Fence*					m_fence;
		HANDLE							m_fenceEvent;
		D3D12_VIEWPORT					m_viewport;
		D3D12_RECT						m_scissorRect;


		/* Engine Modules */
		std::vector<Renderer*>			m_renderers;
		SceneGraph*						m_sceneGraph;
		ResourceManager*				m_resourceManager;

		/* Cyclable Modules */
		std::vector<ICyclable*>			m_cyclables;
		World*							m_world;
		
		bool m_swapChainOccluded = false;
	};
}