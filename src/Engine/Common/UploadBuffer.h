#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <Common/Logger.h>


namespace IHA::Engine {

	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}

	template<typename T>
	class UploadBuffer
	{
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_uploadBuffer;

		BYTE* m_MappedData = nullptr;
		UINT m_ElementByteSize = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptorHandle;

	public:
		UploadBuffer(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle)
		{
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			UINT alignedSize = CalcConstantBufferByteSize(sizeof(T));
			D3D12_RESOURCE_DESC desc = {};
			desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			desc.Alignment = 0;
			desc.Width = alignedSize;
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
				IID_PPV_ARGS(&m_uploadBuffer)
			);

			if (FAILED(hr))
			{
				LOG_ERROR("Create ConstantBuffer Failed.");
			}
			
			m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_MappedData));

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
			cbvDesc.BufferLocation = m_uploadBuffer.Get()->GetGPUVirtualAddress();
			cbvDesc.SizeInBytes = alignedSize;

			m_cpuDescriptorHandle = cpuDescriptorHandle;
			device->CreateConstantBufferView(&cbvDesc, cpuDescriptorHandle);
		}

		void CopyData(int elementIndex, const T& data)
		{
			memcpy(m_MappedData + elementIndex * m_ElementByteSize, &data, sizeof(T));
		}

		ID3D12Resource* Resource() const
		{
			return m_uploadBuffer.Get();
		}
	};
}