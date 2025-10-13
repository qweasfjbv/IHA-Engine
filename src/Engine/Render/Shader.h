#pragma once

#include <wrl.h>
#include <d3d12.h>
#include <string>

using Microsoft::WRL::ComPtr;

namespace IHA::Engine {

    class Shader {
    public:
        ComPtr<ID3DBlob> vertexBlob;
        ComPtr<ID3DBlob> pixelBlob;

        ComPtr<ID3D12PipelineState> pso;
        ComPtr<ID3D12RootSignature> rootSignature;

        bool Load(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device);
        void Bind(ID3D12GraphicsCommandList* cmd);
    };
}