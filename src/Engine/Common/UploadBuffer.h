#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <Common/Logger.h>

template<typename T>
class UploadBuffer
{
private:
	ComPtr<ID3D12Resource> m_UploadBuffer;

	BYTE* m_MappedData = nullptr;
	UINT m_ElementByteSize = 0;
	bool m_IsConstantBuffer = false;

public:

	UploadBuffer(ID3D12Device* device)
	{
		D3D12_HEAP_PROPERTIES heapProps = {};
		heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		HRESULT hr = device->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&buffer)
		);

		if (FAILED(hr))
		{
			LOG_ERROR("Create ConstantBuffer Failed.");
		}

		ThrowIfFailed(
			m_UploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData))
		);
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(m_MappedData + elementIndex * m_ElementByteSize, &data, sizeof(T));
	}

	ID3D12Resource* Resource() const
	{
		return m_UploadBuffer.Get();
	}
};