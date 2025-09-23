#pragma once

#include <d3d12.h>

struct VertexBuffer {
    ID3D12Resource* buffer;
    D3D12_VERTEX_BUFFER_VIEW view;
};

struct IndexBuffer {
    ID3D12Resource* buffer;
    D3D12_INDEX_BUFFER_VIEW view;
    UINT indexCount;
};

