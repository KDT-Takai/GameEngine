#pragma once
#include "Graphics/Component/TransformComponent/TransformComponent.hpp"
#include "System/Screen/Screen.hpp"
#include <DirectXMath.h>

namespace Engine::System::Camera
{
    struct CameraComponent
    {
        enum class ProjectionType
        {
            Orthographic,   // 2Dê≥éÀâe
            Perspective     // 3Dìßéãìäâe
        };

        ProjectionType projectionType = ProjectionType::Orthographic;

        // ã§í ÉpÉâÉÅÅ[É^
        float nearClip = 0.0f;
        float farClip = 1000.0f;
        float zoom = 1.0f;
        float rotation = 0.0f;

        // ìßéãìäâeóp
        float fov = 60.0f;  // éãñÏäp

        // ViewçsóÒéÊìæ
        DirectX::XMMATRIX GetViewMatrix(
            const Engine::Graphics::TransformComponent& transform) const
        {
            DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&transform.position);

            if (projectionType == ProjectionType::Orthographic)
            {
                DirectX::XMVECTOR at = DirectX::XMVectorAdd(
                    eye, DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
                DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

                // rotation
                DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationZ(rotation);
                up = DirectX::XMVector3TransformNormal(up, rotMat);

                return DirectX::XMMatrixLookAtLH(eye, at, up);
            }
            else
            {
                DirectX::XMVECTOR at = DirectX::XMVectorAdd(
                    eye, DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f));
                DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

                return DirectX::XMMatrixLookAtLH(eye, at, up);
            }
        }

        // ProjectionçsóÒéÊìæ
        DirectX::XMMATRIX GetProjectionMatrix() const
        {
            if (projectionType == ProjectionType::Orthographic)
            {
                float w = Engine::System::Screen::GetVirtualWidth() / zoom;
                float h = Engine::System::Screen::GetVirtualHeight() / zoom;
                return DirectX::XMMatrixOrthographicLH(w, h, nearClip, farClip);
            }
            else
            {
                float aspectRatio = Engine::System::Screen::GetAspectRatio();
                return DirectX::XMMatrixPerspectiveFovLH(
                    DirectX::XMConvertToRadians(fov),
                    aspectRatio,
                    nearClip,
                    farClip
                );
            }
        }
    };
} // Engine::System::Camera