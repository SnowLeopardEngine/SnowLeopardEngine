#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class Shader;

    class RenderContext
    {
    public:
        RenderContext() = default;

        Ref<Shader> CreateGraphicsProgram(const std::string&                vertPath,
                                        const std::string&                fragPath,
                                        const std::optional<std::string>& geomPath);
    };
} // namespace SnowLeopardEngine