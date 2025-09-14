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

        virtual void Draw(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap);

    protected:

        virtual void OnGUI(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap) = 0;

        std::string m_Name;
    };
}