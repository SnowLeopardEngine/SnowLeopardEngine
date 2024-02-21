#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class ModelLoader
    {
    public:
        static bool LoadModel(const std::filesystem::path& path, Model& model);
    };
} // namespace SnowLeopardEngine