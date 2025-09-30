#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>
#include <Structs/Buffers.h>
#include <Structs/Material.h>

using namespace DirectX; 
using EntityID = uint32_t;

namespace IHA {

    struct Transform {
        XMFLOAT3 position;
        XMFLOAT3 rotation;
        XMFLOAT3 scale;
    };

    struct Entity {
        EntityID id;
    };

    template<typename T>
    struct ComponentStorage {
        std::unordered_map<EntityID, T> data;
    };

    class Scene {

    public:
        Entity createEntity() {
            Entity e{ nextID++ };
            entities.push_back(e);
            return e;
        }

        template<typename T>
        void addComponent(Entity e, const T& component) {
            getStorage<T>().data[e.id] = component;
        }

        template<typename T>
        T* getComponent(Entity e) {
            auto& storage = getStorage<T>().data;
            auto it = storage.find(e.id);
            return (it != storage.end()) ? &it->second : nullptr;
        }

    private:
        EntityID nextID = 1;
        std::vector<Entity> entities;

        ComponentStorage<Transform> transforms;
        // ComponentStorage<Model> models;

        template<typename T>
        ComponentStorage<T>& getStorage();

    };
}