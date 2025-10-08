#pragma once

#include <d3d12.h>
#include <Core/World.h>

#include <ImGUI/imgui_impl_dx12.h>

namespace IHA::Engine {

    class Camera;

    struct DescHandles
    {
        D3D12_CPU_DESCRIPTOR_HANDLE m_rtvCPUDescHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_rtvGPUDescHandle = {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_srvCPUDescHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_srvGPUDescHandle = {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_dsvCPUDescHandle = {};
        D3D12_GPU_DESCRIPTOR_HANDLE m_dsvGPUDescHandle = {};
    };

    class Renderer
    {
    public:
        Renderer(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, DescHandles&& descHandles)
            : m_device(device), m_commandList(commandList), m_descHandles(std::move(descHandles)) {
        };

        void Resize(UINT width, UINT height);
        void Render(World* world, Camera* camera);

        FORCEINLINE D3D12_CPU_DESCRIPTOR_HANDLE GetRtvCpuDescHandle() const { return m_descHandles.m_rtvCPUDescHandle; }
        FORCEINLINE D3D12_GPU_DESCRIPTOR_HANDLE GetSrvGpuDescHandle() const { return m_descHandles.m_srvGPUDescHandle; }

    private:
        ID3D12Device*               m_device = nullptr;
        ID3D12GraphicsCommandList*  m_commandList = nullptr;
        ID3D12Resource*             m_renderTexture = nullptr;
        DescHandles                 m_descHandles = {};

        UINT                        m_width = 1280;
        UINT                        m_height = 720;
    };

}