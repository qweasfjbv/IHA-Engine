#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <Structs/Buffers.h>
#include <Structs/Material.h>

using namespace DirectX;

namespace IHA {

    struct Transform {
        XMFLOAT3 position;
        XMFLOAT3 rotation;
        XMFLOAT3 scale;
    };

    struct MeshRenderer {
        VertexBuffer vb;
        IndexBuffer ib;
        Material* material;
    };

    struct SceneObject {
        Transform transform;
        MeshRenderer* renderer;
        std::vector<SceneObject*> children;
    };

    class Scene {
        std::vector<std::unique_ptr<SceneObject>> objects;
    public:
        void AddObject(SceneObject* obj, SceneObject* parent);
        void RemoveObject(SceneObject* obj);
        const std::vector<std::unique_ptr<SceneObject>>& GetObjects() const;
    };
}