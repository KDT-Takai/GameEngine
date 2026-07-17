#pragma once
#include "../SpriteRenderer/SpriteRenderer.hpp"
#include "Graphics/Texture/TextureManager/TextureManager.hpp"
#include "Graphics/Component/TransformComponent/TransformComponent.hpp"
#include "Graphics/Component/SpriteComponent/SpriteComponent.hpp"
#include <entt/entt.hpp>
#include <DirectXMath.h>

namespace Engine::Graphics
{
    class SpriteRenderSystem
    {
    public:
        void Update(
            entt::registry& registry,
            ID3D12GraphicsCommandList* cmdList,
            SpriteRenderer& renderer,
            TextureManager& texManager,
            const DirectX::XMMATRIX& view,
            const DirectX::XMMATRIX& projection
        );
    };
} // Engine::Graphics