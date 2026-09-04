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

                // オフセット計算
                float offsetX = -(sprite.pivot.x - 0.5f) * sprite.size.x;
                float offsetY = (sprite.pivot.y - 0.5f) * sprite.size.y;

                // World行列の計算
                DirectX::XMMATRIX world =
                    DirectX::XMMatrixScaling(sprite.size.x, sprite.size.y, 1.0f) *
                    DirectX::XMMatrixTranslation(offsetX, offsetY, 0.0f) *
                    DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z) *
                    DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
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