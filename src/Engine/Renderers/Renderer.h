#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
using Microsoft::WRL::ComPtr;

namespace IHA::Engine {

	class Renderer {

	public:
		inline Renderer(ID3D12Device* device, UINT w, UINT h) {
			Resize(device, w, h);
		}
		~Renderer() = default;

		void Resize(ID3D12Device* device, UINT w, UINT h);
		void RenderFrame(ID3D12GraphicsCommandList* cmd);

		UINT GetWidth() const { return m_Width; }
		UINT GetHeight() const { return m_Height; }

		ComPtr<ID3D12DescriptorHeap> GetSrvHeap() const { return m_SrvHeap; }
		void CopySRVToHeap(ID3D12Device* device, ID3D12DescriptorHeap* dstHeap, UINT slotIndex);

	private:
		ComPtr<ID3D12Resource> m_SceneRenderTarget;
		ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
		ComPtr<ID3D12DescriptorHeap> m_SrvHeap;

		DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		UINT m_Width = 1280;
		UINT m_Height = 720;
	};
}