#pragma once

#include "Render/Shader.h"
#include "Render/Texture.h"

namespace IHA::Engine {

    class Material {
    public:
        Shader* shader = nullptr;            
        std::vector<Texture*> textures;      
        ComPtr<ID3D12DescriptorHeap> cbvSrvHeap;
        XMFLOAT4 colorTint = { 1,1,1,1 };       
    };
}