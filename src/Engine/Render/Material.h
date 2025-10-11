#pragma once

namespace IHA::Engine {

    class Material {
    public:
        Shader* shader = nullptr;            
        std::vector<Texture*> textures;      
        ComPtr<ID3D12DescriptorHeap> cbvSrvHeap;
        XMFLOAT4 colorTint = { 1,1,1,1 };       
    };

    class Shader {
    public:
        ComPtr<ID3DBlob> vertexBlob;
        ComPtr<ID3DBlob> pixelBlob;

        ComPtr<ID3D12PipelineState> pso;  // 파이프라인 상태 객체
        ComPtr<ID3D12RootSignature> rootSignature;

        bool Load(const std::wstring& vsPath, const std::wstring& psPath, ID3D12Device* device);
        void Bind(ID3D12GraphicsCommandList* cmd);
    };

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