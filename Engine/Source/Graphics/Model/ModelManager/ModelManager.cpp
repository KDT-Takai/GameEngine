#include "pch/pch.h"
#include "ModelManager.hpp"
#include "Utility/EngineContext/EngineContext.hpp"

namespace Engine::Graphics
{
    bool ModelManager::Initialize()
    {
        binaryModelLoader = std::make_unique<BinaryModelLoader>();
        LOG_INFO("ModelManagerの初期化に成功");
        return true;
    }

    void ModelManager::Finalize()
    {
        for (auto& [id, model] : models)
        {
            model->Finalize();
        }
        models.clear();
        binaryModelLoader.reset();
    }

    ModelID ModelManager::Load(const std::wstring& path)
    {
        // ハッシュで ModelID 生成
        ModelID id = std::hash<std::wstring>{}(path);

        // キャッシュ確認
        if (models.contains(id))
        {
            return id;
        }

        // モデルロード(.mdlはAssimp非依存のバイナリローダー)
        auto model = binaryModelLoader->Load(path);
        if (!model)
        {
            LOG_ERROR("ModelManager: モデルのロードに失敗: {}", std::string(path.begin(), path.end()));
            return InvalidModelID;
        }

        LOG_INFO("ModelManager: モデルのロードに成功: {}", std::string(path.begin(), path.end()));

        LOG_DEBUG("Model: {} meshes={}", std::string(path.begin(), path.end()), model->GetMeshes().size());

        models[id] = std::move(model);
        return id;
    }

    const Model* ModelManager::Get(ModelID id) const
    {
        if (id == InvalidModelID) return nullptr;

        auto it = models.find(id);
        if (it == models.end()) return nullptr;

        return it->second.get();
    }

    void ModelManager::AddContext()
    {
        REGISTER_CONTEXT(*this);
    }
} // Engine::Graphics