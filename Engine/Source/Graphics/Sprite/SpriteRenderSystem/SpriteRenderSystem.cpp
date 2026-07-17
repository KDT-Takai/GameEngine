#include "pch/pch.h"
#include "SpriteRenderSystem.hpp"

namespace Engine::Graphics
{
    void SpriteRenderSystem::Update(
        entt::registry& registry,
        ID3D12GraphicsCommandList* cmdList,
        SpriteRenderer& renderer,
        TextureManager& texManager,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection)
    {
        auto spriteView = registry.view<TransformComponent, SpriteComponent>();

        spriteView.each([&](auto entity,
            TransformComponent& transform,
            SpriteComponent& sprite)
            {
                if (!sprite.visible) return;

                // World行列の計算
                DirectX::XMMATRIX world =
                    DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, 1.0f) *
                    DirectX::XMMatrixRotationZ(transform.rotation) *
                    DirectX::XMMatrixTranslation(
                        transform.position.x,
                        transform.position.y,
                        transform.position.z
                    );

                // WVP行列の計算
                DirectX::XMMATRIX wvp = world * view * projection;

                // テクスチャ取得（無効IDならfallbackが返る）
                const Texture* texture = texManager.Get(sprite.textureID);

                // 描画
                renderer.Draw(cmdList, wvp, sprite, texture);
            });
    }
} // Engine::Graphics