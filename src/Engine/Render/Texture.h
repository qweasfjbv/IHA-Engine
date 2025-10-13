#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <string>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace IHA::Engine {

    class Texture {
    public:
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
        std::wstring name;

        bool LoadFromFile(const std::wstring& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* cmd);
        void Bind(ID3D12GraphicsCommandList* cmd, UINT rootParamIndex);
    };
}