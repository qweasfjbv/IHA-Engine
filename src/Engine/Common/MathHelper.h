#pragma once

#include "DirectXMath.h"

using namespace DirectX;

namespace IHA::Engine {

    inline XMFLOAT4X4 MakeIdentity()
    {
        XMFLOAT4X4 mat;
        XMStoreFloat4x4(&mat, XMMatrixIdentity());
        return mat;
    }
}