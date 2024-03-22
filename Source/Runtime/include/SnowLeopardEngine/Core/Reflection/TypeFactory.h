#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Object.h"
#include <memory>

namespace SnowLeopardEngine
{

#define NAME_OF_TYPE(type) #type
#define REGISTER_TYPE_BY_NAME(name) TypeFactory::RegisterType<name>(#name);
#define REGISTER_TYPE(type) TypeFactory::RegisterType<type>(NAME_OF_TYPE(type));

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