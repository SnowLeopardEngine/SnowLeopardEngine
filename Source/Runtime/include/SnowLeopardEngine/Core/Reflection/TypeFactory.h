#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Object.h"
#include <memory>

namespace SnowLeopardEngine
{
#define REGISTER_TYPE(name) TypeFactory::RegisterType<name>(#name);

    namespace TypeFactory
    {
        extern std::unordered_map<std::string, std::function<Ref<Object>()>> g_FactoryMap;

        template<typename T>
        static void RegisterType(const std::string& typeName)
        {
            g_FactoryMap[typeName] = []() -> Ref<Object> { return CreateRef<T>(); };
        }

        template<typename T>
        static Ref<T> CreateObject(const std::string& typeName)
        {
            if (g_FactoryMap.find(typeName) != g_FactoryMap.end())
            {
                return std::dynamic_pointer_cast<T>(g_FactoryMap[typeName]());
            }
            return nullptr;
        }
    } // namespace TypeFactory
} // namespace SnowLeopardEngine