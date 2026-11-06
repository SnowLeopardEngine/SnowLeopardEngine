#pragma once

#include "SnowLeopardEngine/Function/Rendering/Material.h"

namespace SnowLeopardEngine
{
    namespace IO
    {
        Material* Load(const std::filesystem::path& materialPath);

        // Frees every cached material. Pointers returned by Load() are invalid afterwards.
        void ReleaseAllMaterials();

        extern std::unordered_map<std::string, Scope<Material>> g_MaterialCache;
    } // namespace IO
} // namespace SnowLeopardEngine