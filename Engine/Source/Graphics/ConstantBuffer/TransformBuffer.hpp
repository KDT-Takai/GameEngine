#pragma once
#include <DirectXMath.h>

namespace Engine::Graphics
{
    struct TransformBuffer
    {
        DirectX::XMFLOAT4X4 wvp;
    };
}