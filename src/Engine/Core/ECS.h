#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <typeindex>

#include "Utils/Types.h"
#include "Common/Interfaces.h"
#include "Common/Logger.h"

namespace IHA::Engine {

    struct Component { };

    template<typename T>
	class SystemBase : public ICyclable {
        static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");

    public:
        virtual ~SystemBase() noexcept = default;
        void Add(Entity e, const T& c) { m_data[e] = c; }
        bool Has(Entity e) const { return m_data.find(e) != m_data.end(); }
        T& Get(Entity e) { return m_data.at(e); }
        T* TryGet(Entity e) {
            auto it = m_data.find(e);
            return it != m_data.end() ? &it->second : nullptr;
        }

        void Update(float deltaTime) override { };

    private:
        std::unordered_map<Entity, T> m_data;
	};

    class EntityManager {
    public:
        inline static Entity g_nextEntityId = 0;
        Entity CreateEntity() { return g_nextEntityId++; }
    };

    class SystemManager : public ICyclable {
    public:
        template<typename T>
        SystemBase<T>& GetPool() {
            std::type_index type = typeid(T);
            if (pools.find(type) != pools.end())
                pools[type] = std::make_shared<SystemBase<T>>();
            return *std::static_pointer_cast<SystemBase<T>>(pools[type]);
        }

        template<typename T>
        void AddComponent(Entity e, const T& comp) {
            GetPool<T>().Add(e, comp);
        }

        template<typename T>
        bool HasComponent(Entity e) {
            return GetPool<T>().Has(e);
        }

        template<typename T>
        T& GetComponent(Entity e) {
            return GetPool<T>().Get(e);
        }

        template<typename T>
        T* TryGetComponent(Entity e) {
            return GetPool<T>().TryGet(e);
        }

        void Update(float deltaTime) override {
            for (auto& [type, pool] : pools)
            {
                auto cycle = std::static_pointer_cast<ICyclable>(pool);
                cycle->Update(deltaTime);
            }
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<void>> pools;
    };

}

