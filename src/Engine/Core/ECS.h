#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <typeindex>

#include "Utils/Types.h"
#include "Common/Interfaces.h"
#include "Common/Logger.h"

namespace IHA::Engine {

	class Component : public ICyclable
	{
	public:
		virtual ~Component() = default;
	};

    class SystemBase
    {
    public:
        virtual ~SystemBase() = default;
        virtual void Update(float deltaTime) = 0;
    };

    template<typename T>
	class ComponentPool : public ICyclable {
        static_assert(std::is_base_of_v<Component, T>, "T must inherit from Component");

    public:
        void Add(Entity e, const T& c) { m_data[e] = c; }
        bool Has(Entity e) const { return m_data.find(e) != m_data.end(); }
        T& Get(Entity e) { return m_data.at(e); }

        void Update(float deltaTime) override {
            for (auto& [entity, comp] : m_data) {
                comp.Update(deltaTime);
            }
        }
    private:
        std::unordered_map<Entity, T> m_data;
	};

    class EntityManager {
    public:
        inline static Entity g_nextEntityId = 0;
        Entity CreateEntity() { return g_nextEntityId++; }
    };

    class ComponentManager : public ICyclable {
    public:
        template<typename T>
        ComponentPool<T>& GetPool() {
            std::type_index type = typeid(T);
            if (pools.find(type) != pools.end())
                pools[type] = std::make_shared<ComponentPool<T>>();
            return *std::static_pointer_cast<ComponentPool<T>>(pools[type]);
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

