#include "Renderer.h"
#include <stdexcept>

namespace IHA::Engine {

	void Renderer::Resize(ID3D12Device* device, UINT w, UINT h) {

		if (!device) return;
		if (w == m_Width && h == m_Height && m_SceneRenderTarget) return;

		m_Width = w;
		m_Height = h;

		// Reset previous resources
		m_SceneRenderTarget.Reset();
		m_RtvHeap.Reset();
		m_SrvHeap.Reset();

		// Create RenderTarget Resource
		D3D12_RESOURCE_DESC texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = m_Width;
		texDesc.Height = m_Height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = m_Format;
		texDesc.SampleDesc.Count = 1;
		texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = m_Format;
		clearValue.Color[0] = .1f;
		clearValue.Color[1] = .1f;
		clearValue.Color[2] = .1f;
		clearValue.Color[3] = 1.0f;

		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			&clearValue,
			IID_PPV_ARGS(&m_SceneRenderTarget));

		if (FAILED(hr))
			throw std::runtime_error("Failed to create SceneView render target");

		// Create RTV Heap
		D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
		rtvDesc.NumDescriptors = 1;
		rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&m_RtvHeap));
		device->CreateRenderTargetView(
			m_SceneRenderTarget.Get(),
			nullptr,
			m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create SRV Heap
		D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
		srvDesc.NumDescriptors = 1;
		srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		device->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&m_SrvHeap));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvViewDesc = {};
		srvViewDesc.Format = m_Format;
		srvViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvViewDesc.Texture2D.MipLevels = 1;
		srvViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		device->CreateShaderResourceView(
			m_SceneRenderTarget.Get(),
			&srvViewDesc,
			m_SrvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void Renderer::RenderFrame(ID3D12GraphicsCommandList* cmd) {

		if (!cmd || !m_SceneRenderTarget) return;

		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();

		D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)m_Width, (float)m_Height, 0.0f, 1.0f };
		cmd->RSSetViewports(1, &viewport);
		D3D12_RECT scissorRect = { 0, 0, (LONG)m_Width, (LONG)m_Height };
		cmd->RSSetScissorRects(1, &scissorRect);

		D3D12_RESOURCE_BARRIER rtBarrier = {};
		rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rtBarrier.Transition.pResource = m_SceneRenderTarget.Get();
		rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		cmd->ResourceBarrier(1, &rtBarrier);
		cmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

		FLOAT clearColor[4] = { .5f, .5f, .1f, 1.0f };
		cmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		D3D12_RESOURCE_BARRIER psBarrier = {};
		psBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		psBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		psBarrier.Transition.pResource = m_SceneRenderTarget.Get();
		psBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		psBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		psBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		cmd->ResourceBarrier(1, &psBarrier);
	}

	void Renderer::CopySRVToHeap(ID3D12Device* device, ID3D12DescriptorHeap* dstHeap, UINT slotIndex)
	{
		UINT descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = dstHeap->GetCPUDescriptorHandleForHeapStart();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = dstHeap->GetGPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += slotIndex * descriptorSize;
		gpuHandle.ptr += slotIndex * descriptorSize;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		device->CreateShaderResourceView(m_SceneRenderTarget.Get(), &srvDesc, cpuHandle);
	}
}