#include "pch/pch.h"
#include "ModelRenderSystem.hpp"

namespace Engine::Graphics
{
    void ModelRenderSystem::Update(
        entt::registry& registry,
        ID3D12GraphicsCommandList* cmdList,
        ModelRenderer& renderer,
        ModelManager& modelManager,
        TextureManager& texManager,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection)
    {
        auto modelView = registry.view<TransformComponent, ModelComponent>();

        modelView.each([&](auto entity, TransformComponent& transform, ModelComponent& model)
            {

                if (!model.visible) return;

                //LOG_DEBUG("ÉÇÉfÉãï`âÊ: modelID={}", static_cast<uint64_t>(model.modelID));

                const Model* modelData = modelManager.Get(model.modelID);
                
                // LOG_DEBUG("ModelRenderSystem: entityï`âÊäJén modelID={}", static_cast<uint64_t>(model.modelID));
                // LOG_DEBUG("ModelRenderSystem: modelData={}", (void*)modelData);
                // LOG_DEBUG("ModelRenderSystem: visible={}", model.visible);

                if (!modelData)
                {
                    LOG_ERROR("ModelÇ™éÊìæÇ≈Ç´Ç»Ç¢ ModelID={}",
                        static_cast<uint64_t>(model.modelID));
                    return;
                }

                // LOG_DEBUG("Model Transform: pos=({}, {}, {}), scale=({}, {}, {})", transform.position.x, transform.position.y, transform.position.z, transform.scale.x, transform.scale.y, transform.scale.z);

                // WorldçsóÒÇÃåvéZ
                DirectX::XMMATRIX world =
                    DirectX::XMMatrixScaling(transform.scale.x, transform.scale.y, transform.scale.z) *
                    DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z) *
                    DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z);

                renderer.Draw(cmdList, modelData, world, view, projection, texManager);
            });
    }
} // Engine::Graphics