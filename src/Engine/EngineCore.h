#include <vector>
#include <d3d12.h>
#include <dxgi1_6.h>


namespace IHA::Engine {

	/* Forward Declarations */
	class Renderer;
	class SceneGraph;
	class ResourceManager;

	struct FrameContext
	{
		ID3D12CommandAllocator*		commandAllocator;
		UINT64                      fenceValue;
	};

	struct DescriptorHeapAllocator
	{
	public:
		DescriptorHeapAllocator(ID3D12DescriptorHeap* heap, UINT size, UINT descSize)
			: m_heap(heap), m_size(size), m_nextFree(0), m_descriptorSize(descSize) {}

		void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
		{
			// if (m_nextFree >= m_size) return false;

			outCpu->ptr = m_heap->GetCPUDescriptorHandleForHeapStart().ptr + m_nextFree * m_descriptorSize;
			outGpu->ptr = m_heap->GetGPUDescriptorHandleForHeapStart().ptr + static_cast<UINT64>(m_nextFree) * m_descriptorSize;

			++m_nextFree;
			// return true;
		}

		void Free(D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu)
		{
			if (m_nextFree > 0) --m_nextFree;
			// return true;
		}

	private:
		ID3D12DescriptorHeap* m_heap;
		UINT m_size;
		UINT m_nextFree;
		UINT m_descriptorSize = 0;
	};

	class EngineCore {

	public:
		bool Init(HWND hwnd);
		void PreUpdate();
		void Update();
		void PostUpdate();
		void BeginFrame();
		void EndFrame();
		void Render();
		void Present();

		void ShutDown();
		void WaitForLastSubmittedFrame();
		bool IsSwapChainOccluded();

	private:
		bool CreateDeviceD3D(HWND hwnd);
		void CleanupDeviceD3D();
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

		FORCEINLINE Renderer* GetRenderer() const { return m_renderer; }
		FORCEINLINE SceneGraph* GetScene() const { return m_sceneGraph; }
		FORCEINLINE ResourceManager* GetResourceManager() const { return m_resourceManager; }

		static const int APP_NUM_FRAMES_IN_FLIGHT = 2;
		static const int APP_NUM_BACK_BUFFERS = 2;
		static const int APP_SRV_HEAP_SIZE = 64;

		static DescriptorHeapAllocator* g_srvDescHeapAlloc;

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
		Renderer*						m_renderer;
		SceneGraph*						m_sceneGraph;
		ResourceManager*				m_resourceManager;
	};
}