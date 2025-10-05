#pragma once

namespace IHA::Engine {

    struct ICyclable {
        // virtual void PreUpdate(float deltaTime);
        virtual void Update(float deltaTime) = 0;
        // virtual void PostUpdate(float deltaTime);
        virtual ~ICyclable() = default;
    };



}