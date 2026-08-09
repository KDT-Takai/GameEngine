#pragma once
#include <DirectXMath.h>

namespace Engine::Graphics
{
    struct TransformComponent
    {
        DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
        float             rotation = 0.0f;
        DirectX::XMFLOAT2 scale = { 1.0f, 1.0f };
    };
}