#include "pch/pch.h"
#include "CameraSystem.hpp"

namespace Engine::System::Camera
{
    CameraData CameraSystem::GetMainCameraData(entt::registry& registry) const
    {
        // デフォルト値（カメラが見つからない場合）
        CameraData data;
        data.view = DirectX::XMMatrixIdentity();
        data.projection = DirectX::XMMatrixOrthographicLH(
            Engine::System::Screen::GetVirtualWidth(),
            Engine::System::Screen::GetVirtualHeight(),
            0.0f,
            1000.0f
        );

        // MainCameraTag がついたエンティティを検索
        auto view = registry.view<CameraComponent, Engine::Graphics::TransformComponent, Engine::System::ECS::MainCameraTag>();

        bool found = false;
        view.each([&](auto entity,
            CameraComponent& camera,
            Engine::Graphics::TransformComponent& transform)
            {
                if (found) return;

                data.view = camera.GetViewMatrix(transform);
                data.projection = camera.GetProjectionMatrix();
                found = true;
            });

		// MainCameraTag を持つカメラが見つからなかった場合、警告ログを出力
        static bool logged = false;
        if (!found && !logged)
        {
            LOG_WARN("MainCameraTagを持つカメラが見つかりません。デフォルト値を使用します。");
            logged = true;
        }

        return data;
    }
} // Engine::System::Camera