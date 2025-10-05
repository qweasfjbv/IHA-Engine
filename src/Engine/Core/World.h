#pragma once

#include "Core/ECS.h"

namespace IHA::Engine {

    class World : public ICyclable {
        EntityManager entityMgr;
        ComponentManager compMgr;
    public:
        Entity CreateEntity() { return entityMgr.CreateEntity(); }

        template<typename T, typename... Args>
        void AddComponent(Entity e, Args&&... args) {
            compMgr.AddComponent<T>(e, T(std::forward<Args>(args)...));
        }

        template<typename T>
        bool HasComponent(Entity e) { return compMgr.HasComponent<T>(e); }

        template<typename T>
        T& GetComponent(Entity e) { return compMgr.GetComponent<T>(e); }

        void Update(float deltaTime) override
        {
            compMgr.Update(deltaTime);
        }
    };
}