#pragma once

#include "Render/Shader.h"
#include "Render/Texture.h"

namespace IHA::Engine {

    class Material {
    public:
        Shader* shader = nullptr;            
        Texture* texture = nullptr;      
        ComPtr<ID3D12DescriptorHeap> cbvSrvHeap;
        XMFLOAT4 colorTint = { 1,1,1,1 };

        void Bind(ID3D12GraphicsCommandList* cmd) {
            shader->Bind(cmd);
            // HACK
            texture->Bind(cmd, 1);
        }
    };
}