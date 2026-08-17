#pragma once
#include "Graphics/Model/ModelID/ModelID.hpp"

namespace Engine::Graphics
{
    struct ModelComponent
    {
        ModelID modelID = InvalidModelID;
        bool    visible = true;

        // アニメーション関連
        int   animationIndex = 0;
        float animationTime = 0.0f;
        bool  isPlaying = false;
        bool  isLoop = true;
        float animationSpeed = 1.0f;

        // 描画関連
        bool castShadow = true;
        bool receiveShadow = true;

        // レイヤー
        int layer = 0;
    };
} // Engine::Graphics