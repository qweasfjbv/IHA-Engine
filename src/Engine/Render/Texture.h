#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <string>

using Microsoft::WRL::ComPtr;

namespace IHA::Engine {

    class Texture {
    public:
        Texture(const std::wstring& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* cmd, 
            D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle);

        bool LoadFromFile(const std::wstring& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* cmd);
        void Bind(ID3D12GraphicsCommandList* cmd, UINT rootParamIndex);

        ComPtr<ID3D12Resource> m_resource;
        D3D12_CPU_DESCRIPTOR_HANDLE m_srvCpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE m_srvGpuHandle;
        std::wstring m_name;
    };
}