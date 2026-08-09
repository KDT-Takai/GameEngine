#pragma once
#include "Graphics/Texture/TextureID/TextureID.hpp"
#include <DirectXMath.h>

namespace Engine::Graphics
{
    struct SpriteComponent
    {
        TextureID           textureID = InvalidTextureID;
        DirectX::XMFLOAT2   size = { 100.0f, 100.0f }; // ピクセル単位
        DirectX::XMFLOAT2   pivot = { 0.5f, 0.5f }; // 正規化（中央が0.5, 0.5）
		DirectX::XMFLOAT4   color = { 1.0f, 1.0f, 1.0f, 1.0f }; // RGBA
        DirectX::XMFLOAT4   uvRect = { 0.0f, 0.0f, 1.0f, 1.0f }; // { u, v, w, h }
        bool                visible = true;
    };
}