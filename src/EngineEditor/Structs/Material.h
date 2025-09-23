#pragma once

#include <d3d12.h>

class Material {
public:
    ID3D12PipelineState* pso;
    ID3D12RootSignature* rootSig;
    D3D12_GPU_DESCRIPTOR_HANDLE srvHandle;
    D3D12_GPU_VIRTUAL_ADDRESS cbAddress;
};