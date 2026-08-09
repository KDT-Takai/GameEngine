#pragma once
#include "Utility/Singleton/Singleton.hpp"
#include "Utility/Logger/Log.hpp"
#include <unordered_map>
#include <typeindex>

// É}ÉNÉç
#define ENGINE_CONTEXT()       Engine::Utility::EngineContext::GetInstance()
#define GET_CONTEXT(Type)      Engine::Utility::EngineContext::GetInstance().Get<Type>()
#define REGISTER_CONTEXT(inst) Engine::Utility::EngineContext::GetInstance().Register(inst)

namespace Engine::Utility
{
    class EngineContext : public Singleton<EngineContext>
    {
        DECLARE_SINGLETON(EngineContext)
    public:
        template<typename T>
        void Register(T& instance)
        {
            auto key = std::type_index(typeid(T));
            services[key] = &instance;
            LOG_DEBUG("Registered service: {}", typeid(T).name());
        }

        template<typename T>
        T* Get()
        {
            auto key = std::type_index(typeid(T));
            auto it = services.find(key);
            if (it == services.end()) {
                LOG_ERROR("ServiceÇ™å©Ç¬Ç©ÇËÇ‹ÇπÇÒ");
                return nullptr;
            }
            return static_cast<T*>(it->second);
        }

    private:
        std::unordered_map<std::type_index, void*> services;
    };
} // Engine::Utility