#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <vector>
#include "DirectXMath.h"
#include "Common/Logger.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace IHA::Engine {

    struct Vertex {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT2 uv;
    };

    class Mesh {
    public:
        void InitBuffers(ID3D12Device* dev, ID3D12GraphicsCommandList* cmd,
            const std::vector<Vertex>& vertices,
            const std::vector<uint32_t>& indices) {
            UINT vbByteSize = (UINT)(vertices.size() * sizeof(Vertex));
            UINT ibByteSize = (UINT)(indices.size() * sizeof(uint32_t));
            indexCount = indices.size();

            D3D12_HEAP_PROPERTIES defaultHeapProps = {};
            defaultHeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
            defaultHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            defaultHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            defaultHeapProps.CreationNodeMask = 1;
            defaultHeapProps.VisibleNodeMask = 1;

            D3D12_HEAP_PROPERTIES uploadHeapProps = {};
            uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            uploadHeapProps.CreationNodeMask = 1;
            uploadHeapProps.VisibleNodeMask = 1;

            D3D12_RESOURCE_DESC vbDesc = {};
            vbDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            vbDesc.Alignment = 0;
            vbDesc.Width = vbByteSize;
            vbDesc.Height = 1;
            vbDesc.DepthOrArraySize = 1;
            vbDesc.MipLevels = 1;
            vbDesc.Format = DXGI_FORMAT_UNKNOWN;
            vbDesc.SampleDesc.Count = 1;
            vbDesc.SampleDesc.Quality = 0;
            vbDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            vbDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

            HRESULT hr = dev->CreateCommittedResource(
                &defaultHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &vbDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_vertexBufferGPU));
            if (FAILED(hr)) Logger::Log("Failed to create vertex buffer (GPU)", LogLevel::Error);

            hr = dev->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &vbDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_vertexBufferUploader));
            if (FAILED(hr)) Logger::Log("Failed to create vertex buffer (Upload)", LogLevel::Error);

            void* mappedData = nullptr;
            D3D12_RANGE range = { 0, 0 };
            m_vertexBufferUploader->Map(0, &range, &mappedData);
            memcpy(mappedData, vertices.data(), vbByteSize);
            m_vertexBufferUploader->Unmap(0, nullptr);
            
            cmd->CopyResource(m_vertexBufferGPU.Get(), m_vertexBufferUploader.Get());

            D3D12_RESOURCE_DESC ibDesc = vbDesc;
            ibDesc.Width = ibByteSize;

            hr = dev->CreateCommittedResource(
                &defaultHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &ibDesc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                nullptr,
                IID_PPV_ARGS(&m_indexBufferGPU));
            if (FAILED(hr)) Logger::Log("Failed to create index buffer (GPU)", LogLevel::Error);

            hr = dev->CreateCommittedResource(
                &uploadHeapProps,
                D3D12_HEAP_FLAG_NONE,
                &ibDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_indexBufferUploader));
            if (FAILED(hr)) Logger::Log("Failed to create index buffer(Upload)", LogLevel::Error);

            m_indexBufferUploader->Map(0, &range, &mappedData);
            memcpy(mappedData, indices.data(), ibByteSize);
            m_indexBufferUploader->Unmap(0, nullptr);

            cmd->CopyResource(m_indexBufferGPU.Get(), m_indexBufferUploader.Get());

            D3D12_RESOURCE_BARRIER barriers[2] = {};

            barriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[0].Transition.pResource = m_vertexBufferGPU.Get();
            barriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            barriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            barriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barriers[1].Transition.pResource = m_indexBufferGPU.Get();
            barriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;
            barriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

            cmd->ResourceBarrier(2, barriers);

            m_vertexBufferView.BufferLocation = m_vertexBufferGPU->GetGPUVirtualAddress();
            m_vertexBufferView.StrideInBytes = sizeof(float) * 3;
            m_vertexBufferView.SizeInBytes = vbByteSize;

            m_indexBufferView.BufferLocation = m_indexBufferGPU->GetGPUVirtualAddress();
            m_indexBufferView.SizeInBytes = ibByteSize;
            m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        }

        void Draw(ID3D12GraphicsCommandList* cmd) const {
            cmd->IASetVertexBuffers(0, 1, &m_vertexBufferView);
            cmd->IASetIndexBuffer(&m_indexBufferView);
            cmd->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
        }

    private:
        ComPtr<ID3D12Resource> m_vertexBufferGPU;
        ComPtr<ID3D12Resource> m_vertexBufferUploader;
        ComPtr<ID3D12Resource> m_indexBufferGPU;
        ComPtr<ID3D12Resource> m_indexBufferUploader;

        D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView{};
        D3D12_INDEX_BUFFER_VIEW m_indexBufferView{};
        UINT indexCount = 0;
    };
}