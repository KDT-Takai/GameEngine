#pragma once
#include <DirectXMath.h>

namespace Engine::Graphics
{
    struct ModelVertex
    {
        DirectX::XMFLOAT3 position;   // 位置
        DirectX::XMFLOAT3 normal;     // 法線
        DirectX::XMFLOAT2 uv;         // テクスチャ座標
        DirectX::XMFLOAT3 tangent;    // 接線
        DirectX::XMFLOAT3 bitangent;  // 従法線
    };
} // Engine::Graphics