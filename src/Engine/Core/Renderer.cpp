#include "Renderer.h"
#include "Components/Camera.h"

namespace IHA::Engine {

    void Renderer::Resize(UINT width, UINT height)
    {
        if (m_width == width && m_height == height) return;
        m_width = width; m_height = height;

        if (m_renderTexture)
        {
            m_renderTexture->Release();
            m_renderTexture = nullptr;
        }

        D3D12_RESOURCE_DESC desc = {};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        desc.Width = width;
        desc.Height = height;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

        D3D12_CLEAR_VALUE clearValue{};
        clearValue.Format = desc.Format;
        clearValue.Color[0] = 0.1f;
        clearValue.Color[1] = 0.1f;
        clearValue.Color[2] = 0.1f;
        clearValue.Color[3] = 1.0f;

        D3D12_HEAP_PROPERTIES heapProps{};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
        heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProps.CreationNodeMask = 1;
        heapProps.VisibleNodeMask = 1;

        HRESULT hr = m_device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &clearValue,
            IID_PPV_ARGS(&m_renderTexture));

        assert(m_renderTexture != nullptr);
        if (FAILED(hr)) {
            LOG_INFO("Renderer Create Resource Failed");
            return;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvViewDesc = {};
        srvViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvViewDesc.Texture2D.MipLevels = 1;
        srvViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        m_device->CreateRenderTargetView(m_renderTexture, nullptr, m_descHandles.m_rtvCPUDescHandle);
        m_device->CreateShaderResourceView(m_renderTexture, &srvViewDesc, m_descHandles.m_srvCPUDescHandle);
    }

    void Renderer::Render(World* world)
	{
        D3D12_RESOURCE_BARRIER rtBarrier = {};
        rtBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        rtBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        rtBarrier.Transition.pResource = m_renderTexture;
        rtBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        rtBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        rtBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        m_commandList->ResourceBarrier(1, &rtBarrier);

        float clearColor[4] = { 0.2f, 0.3f, 0.8f, 1.0f };
        
        m_commandList->OMSetRenderTargets(
            1,
            &m_descHandles.m_rtvCPUDescHandle,
            FALSE,
            nullptr
        );
        m_commandList->ClearRenderTargetView(m_descHandles.m_rtvCPUDescHandle, clearColor, 0, nullptr);

       D3D12_RESOURCE_BARRIER psBarrier = {};
       psBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
       psBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
       psBarrier.Transition.pResource = m_renderTexture;
       psBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
       psBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
       psBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
       m_commandList->ResourceBarrier(1, &psBarrier);
	}
}