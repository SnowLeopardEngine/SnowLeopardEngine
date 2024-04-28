#pragma once

#include "SnowLeopardEngine/Function/Rendering/Material.h"

namespace SnowLeopardEngine
{
    namespace IO
    {
        Material* Load(const std::filesystem::path& materialPath);

        extern std::unordered_map<std::string, Material*> g_MaterialCache;
    } // namespace IO
} // namespace SnowLeopardEngine