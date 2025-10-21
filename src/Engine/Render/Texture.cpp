#include "Texture.h"
#include "Common/Logger.h"
#include "d3d12.h"
#include <wincodec.h>
#pragma comment(lib, "windowscodecs.lib")

namespace IHA::Engine {

    UINT64 GetRequiredIntermediateSize_Manual(
        ID3D12Device* device,
        ID3D12Resource* resource,
        UINT firstSubresource,
        UINT numSubresources);

    bool Texture::LoadFromFile(const std::wstring& filePath, ID3D12Device* device, ID3D12GraphicsCommandList* cmd, ID3D12DescriptorHeap* srvHeap, UINT descriptorIndex)
    {
        HRESULT hr;
        
        ComPtr<IWICImagingFactory> wicFactory;
        hr = CoCreateInstance(
            CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wicFactory));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create WIC factory");
            return false;
        }

        ComPtr<IWICBitmapDecoder> decoder;
        hr = wicFactory->CreateDecoderFromFilename(
            filePath.c_str(), nullptr, GENERIC_READ,
            WICDecodeMetadataCacheOnDemand, &decoder);
        if (FAILED(hr)) {
            LOG_ERROR("Failed to load image file");
            return false;
        }

        ComPtr<IWICBitmapFrameDecode> frame;
        decoder->GetFrame(0, &frame);

        ComPtr<IWICFormatConverter> converter;
        wicFactory->CreateFormatConverter(&converter);
        converter->Initialize(
            frame.Get(), GUID_WICPixelFormat32bppRGBA,
            WICBitmapDitherTypeNone, nullptr, 0.0f,
            WICBitmapPaletteTypeCustom);

        UINT width, height;
        converter->GetSize(&width, &height);
        UINT stride = width * 4;
        UINT imageSize = stride * height;

        std::vector<BYTE> imageData(imageSize);
        converter->CopyPixels(nullptr, stride, imageSize, imageData.data());

        D3D12_RESOURCE_DESC texDesc = {};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Alignment = 0;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

        hr = device->CreateCommittedResource(
            &heapProps,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&resource));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create GPU texture resource");
            return false;
        }

        UINT64 uploadBufferSize = GetRequiredIntermediateSize_Manual(device, resource.Get(), 0, 1);
        D3D12_HEAP_PROPERTIES uploadHeapProps = {};
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = uploadBufferSize;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        bufferDesc.SampleDesc.Count = 1;

        ComPtr<ID3D12Resource> uploadBuffer;
        hr = device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer));
        if (FAILED(hr)) {
            LOG_ERROR("Failed to create upload buffer for texture");
            return false;
        }

        D3D12_SUBRESOURCE_DATA textureData = {};
        textureData.pData = imageData.data();
        textureData.RowPitch = stride;
        textureData.SlicePitch = textureData.RowPitch * height;

        {
            BYTE* mappedData;
            D3D12_RANGE range{ 0, 0 };
            uploadBuffer->Map(0, &range, reinterpret_cast<void**>(&mappedData));
            memcpy(mappedData, imageData.data(), imageSize);
            uploadBuffer->Unmap(0, nullptr);

            // 복사 명령 기록
            D3D12_TEXTURE_COPY_LOCATION dst = {};
            dst.pResource = resource.Get();
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = 0;

            D3D12_TEXTURE_COPY_LOCATION src = {};
            src.pResource = uploadBuffer.Get();
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            device->GetCopyableFootprints(&texDesc, 0, 1, 0, &src.PlacedFootprint, nullptr, nullptr, nullptr);

            cmd->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
        }

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = resource.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmd->ResourceBarrier(1, &barrier);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = srvHeap->GetCPUDescriptorHandleForHeapStart();
        D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = srvHeap->GetGPUDescriptorHandleForHeapStart();
        UINT increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        cpuHandle.ptr = cpuStart.ptr + (UINT64)descriptorIndex * increment;
        gpuHandle.ptr = gpuStart.ptr + (UINT64)descriptorIndex * increment;

        device->CreateShaderResourceView(resource.Get(), &srvDesc, cpuHandle);

        name = filePath;
        return true;
    }

    void Texture::Bind(ID3D12GraphicsCommandList* cmd, UINT rootParamIndex)
    {
        if (gpuHandle.ptr == 0)
            return;

        cmd->SetGraphicsRootDescriptorTable(rootParamIndex, gpuHandle);
    }

    UINT64 GetRequiredIntermediateSize_Manual(
        ID3D12Device* device,
        ID3D12Resource* destinationResource,
        UINT firstSubresource,
        UINT numSubresources)
    {
        D3D12_RESOURCE_DESC desc = destinationResource->GetDesc();

        UINT64 totalBytes = 0;
        UINT64 rowSizesInBytes[16];
        UINT numRows[16];
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[16];

        device->GetCopyableFootprints(
            &desc,
            firstSubresource,
            numSubresources,
            0,
            layouts,
            numRows,
            rowSizesInBytes,
            &totalBytes
        );

        return totalBytes;
    }
}