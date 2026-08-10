// Source/System/Camera/CameraSystem/CameraSystem.hpp
#pragma once
#include "System/Camera/CameraComponent/CameraComponent.hpp"
#include "System/ECS/Tag/MainCameraTag.hpp"
#include "Graphics/Component/TransformComponent/TransformComponent.hpp"
#include <entt/entt.hpp>
#include <DirectXMath.h>

namespace Engine::System::Camera
{
    struct CameraData
    {
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX projection;
    };

    class CameraSystem
    {
    public:
        // MainCameraTag がついたカメラの View・Projection 行列を返す
        // 見つからない場合はデフォルト値を返す
        CameraData GetMainCameraData(entt::registry& registry) const;
    };
} // Engine::System::Camera