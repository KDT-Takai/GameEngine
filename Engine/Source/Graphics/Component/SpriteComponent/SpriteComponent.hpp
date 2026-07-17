#pragma once
#include "Graphics/Texture/TextureID/TextureID.hpp"
#include <DirectXMath.h>

namespace Engine::Graphics
{
    struct SpriteComponent
    {
        TextureID           textureID = InvalidTextureID;
        DirectX::XMFLOAT4   color = { 1.0f, 1.0f, 1.0f, 1.0f };
        DirectX::XMFLOAT4   uvRect = { 0.0f, 0.0f, 1.0f, 1.0f }; // { u, v, w, h }
        bool                visible = true;
    };
}