#pragma once

#include <string>
#include "d3d12.h"

namespace IHA::Editor {

    class WindowBase {

    public:
        WindowBase(const char* name)
            : m_Name(name) {}
        virtual ~WindowBase();

        virtual void Update() {}
        virtual void Resize(unsigned int w, unsigned int h) {}

        virtual void Draw();

    protected:

        virtual void OnGUI() = 0;

        std::string m_Name;
    };
}