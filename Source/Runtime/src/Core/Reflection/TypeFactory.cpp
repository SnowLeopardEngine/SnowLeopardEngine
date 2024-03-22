#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"

namespace SnowLeopardEngine
{
    namespace TypeFactory
    {
        std::unordered_map<std::string, std::function<Ref<Object>()>> g_FactoryMap;
    }
} // namespace SnowLeopardEngine