#pragma once

#include "Render/Shader.h"
#include "Render/Texture.h"

namespace IHA::Engine {

    class Material {
    public:
        Shader* shader = nullptr;
        // Texture* texture = nullptr;
        
        void Bind(ID3D12GraphicsCommandList* cmd) {
            shader->Bind(cmd);
            // HACK
            // texture->Bind(cmd, 1);
        }
    };
}