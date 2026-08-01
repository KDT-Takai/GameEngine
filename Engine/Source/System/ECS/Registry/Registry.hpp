// Source/System/ECS/Registry/Registry.hpp
#pragma once
#include "Utility/Singleton/Singleton.hpp"
#include <entt/entt.hpp>
#include <vector>
#include <algorithm>
#include <ranges>

namespace Engine::System::ECS
{
    // 永続タグ
    struct PersistentTag {};

    class Registry : public Engine::Utility::Singleton<Registry>
    {
        DECLARE_SINGLETON(Registry)
    public:
        // エンティティ管理
        entt::entity CreateEntity()
        {
            return registry.create();
        }

        void DestroyEntity(entt::entity entity)
        {
            if (IsValid(entity))
            {
                registry.destroy(entity);
            }
        }

        bool IsValid(entt::entity entity) const
        {
            return registry.valid(entity);
        }

        // 全破棄
        void AllClear()
        {
            std::vector<entt::entity> targets;
            for (auto entity : registry.view<entt::entity>())
            {
                if (!registry.all_of<PersistentTag>(entity))
                {
                    targets.push_back(entity);
                }
            }
            for (auto entity : targets)
            {
                registry.destroy(entity);
            }
        }

        // 全破棄 PersistentTagも含む
        void ForceAllClear()
        {
            registry.clear();
        }

        // AとB両方持つものを消す
        template<typename... Tags>
        void DestroyWithAllTags()
        {
            std::vector<entt::entity> targets;
            auto view = registry.view<Tags...>();
            for (auto entity : view)
            {
                targets.push_back(entity);
            }
            for (auto entity : targets)
            {
                if (IsValid(entity))
                {
                    registry.destroy(entity);
                }
            }
        }

        // AまたはB持つものを消す
        template<typename... Tags>
        void DestroyWithAnyTag()
        {
            std::vector<entt::entity> targets;
            ([&]()
                {
                    auto view = registry.view<Tags>();
                    for (auto entity : view)
                    {
                        targets.push_back(entity);
                    }
                }(), ...);

            // 重複除去
            std::ranges::sort(targets);
            auto [first, last] = std::ranges::unique(targets);
            targets.erase(first, last);

            for (auto entity : targets)
            {
                if (IsValid(entity))
                {
                    registry.destroy(entity);
                }
            }
        }

        // コンポーネント追加
        template<typename T, typename... Args>
        T& AddComponent(entt::entity entity, Args&&... args)
        {
            return registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        // コンポーネント削除
        template<typename T>
        void RemoveComponent(entt::entity entity)
        {
            registry.remove<T>(entity);
        }

        // コンポーネント取得
        template<typename T>
        T& GetComponent(entt::entity entity)
        {
            return registry.get<T>(entity);
        }

        template<typename T>
        const T& GetComponent(entt::entity entity) const
        {
            return registry.get<T>(entity);
        }

        // コンポーネント有無チェック
        template<typename T>
        bool HasComponent(entt::entity entity) const
        {
            return registry.all_of<T>(entity);
        }

        // View取得
        template<typename... Components>
        auto View()
        {
            return registry.view<Components...>();
        }

        // View取得（除外指定あり）
        template<typename... Components, typename... Excludes>
        auto ViewExclude(entt::exclude_t<Excludes...> exclude)
        {
            return registry.view<Components...>(exclude);
        }

        // エンティティ数
        size_t GetEntityCount() const
        {
            return registry.storage<entt::entity>()->size() - registry.storage<entt::entity>()->free_list();
        }

        // レジストリ直接アクセス（緊急用）
        entt::registry& GetRegistry()
        {
            return registry;
        }

    private:
        entt::registry registry;
    };
} // Engine::System::ECS