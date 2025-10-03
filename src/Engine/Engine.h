#include <vector>
#include <d3d12.h>
#include <dxgi1_6.h>

/* Forward Declarations */
class Renderer;
class SceneGraph;
class ResourceManager;

namespace IHA::Engine {

	struct FrameContext
	{
		ID3D12CommandAllocator*		commandAllocator;
		UINT64                      fenceValue;
	};

	class Engine {

	public:
		bool Init(HWND hwnd);
		void Update();
		void Render();
		void ShutDown();

	private:
		bool CreateDeviceD3D(HWND hwnd);
		void CleanupDeviceD3D();
		void WaitForLastSubmittedFrame();
		FrameContext* WaitForNextFrameResources();

	public:
		FORCEINLINE ID3D12Device* GetDevice() const { return m_device; }
		FORCEINLINE ID3D12CommandQueue* GetCommandQueue() const { return m_commandQueue; }

		FORCEINLINE Renderer* GetRenderer() const { return m_renderer; }
		FORCEINLINE SceneGraph* GetScene() const { return m_sceneGraph; }
		FORCEINLINE ResourceManager* GetResourceManager() const { return m_resourceManager; }

	private:

		static const int APP_NUM_FRAMES_IN_FLIGHT = 2;
		static const int APP_NUM_BACK_BUFFERS = 2;
		static const int APP_SRV_HEAP_SIZE = 64;


		/* D3D12 Graphics */
		ID3D12Device*					m_device = nullptr;
		IDXGISwapChain3*				m_swapChain = nullptr;
		HANDLE							m_swapChainWaitableObject = nullptr;
		ID3D12CommandQueue*				m_commandQueue = nullptr;
		ID3D12GraphicsCommandList*		m_commandList = nullptr;
		FrameContext					m_frameContext[APP_NUM_FRAMES_IN_FLIGHT] = {};
		UINT							m_frameIndex = 0;

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