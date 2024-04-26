#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    Ref<Shader> RenderContext::CreateGraphicsProgram(const std::string&                vertSource,
                                                     const std::string&                fragSource,
                                                     const std::optional<std::string>& geomSource)
    {
        // TODO: Geometry, Compute
        return Shader::Create(vertSource, fragSource);
    }
} // namespace SnowLeopardEngine