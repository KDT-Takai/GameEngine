#pragma once
#include "Graphics/Texture/TextureID/TextureID.hpp"
#include <DirectXMath.h>
#include <string>

namespace Engine::Graphics
{
    struct Material
    {
        std::string name = "Default";

        // ベースカラー
        DirectX::XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

        // テクスチャ
        TextureID diffuseTexture = InvalidTextureID;  // 色テクスチャ

        // 将来追加
        // TextureID normalMap    = InvalidTextureID;  // 法線マップ
        // TextureID roughnessMap = InvalidTextureID;  // 粗さ
        // TextureID metallicMap  = InvalidTextureID;  // 金属度
        // float     roughness    = 0.5f;
        // float     metallic     = 0.0f;
    };
} // Engine::Graphics