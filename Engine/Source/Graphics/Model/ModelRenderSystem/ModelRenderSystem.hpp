#pragma once
#include "Graphics/Model/ModelRenderer/ModelRenderer.hpp"
#include "Graphics/Model/ModelManager/ModelManager.hpp"
#include "Graphics/Component/TransformComponent/TransformComponent.hpp"
#include "Graphics/Component/ModelComponent/ModelComponent.hpp"
#include <entt/entt.hpp>
#include <DirectXMath.h>

namespace Engine::Graphics
{
    class ModelRenderSystem
    {
    public:
        void Update(
            entt::registry& registry,
            ID3D12GraphicsCommandList* cmdList,
            ModelRenderer& renderer,
            ModelManager& modelManager,
            TextureManager& texManager,
            const DirectX::XMMATRIX& view,
            const DirectX::XMMATRIX& projection
        );
    };
} // Engine::Graphics