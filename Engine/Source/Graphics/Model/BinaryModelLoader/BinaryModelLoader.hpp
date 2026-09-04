#pragma once
#include "Graphics/Model/Model/Model.hpp"
#include <memory>
#include <string>

namespace Engine::Graphics
{
    class BinaryModelLoader
    {
    public:
        // ModelConverterが出力した.mdlバイナリを読み込みModelを返す
        std::unique_ptr<Model> Load(const std::wstring& path);
    };
} // Engine::Graphics
