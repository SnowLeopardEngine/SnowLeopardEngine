#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class Shader;

    class RenderContext
    {
    public:
        RenderContext() = default;

        Ref<Shader> CreateGraphicsProgram(const std::string&                vertSource,
                                          const std::string&                fragSource,
                                          const std::optional<std::string>& geomSource = std::nullopt);
    };
} // namespace SnowLeopardEngine