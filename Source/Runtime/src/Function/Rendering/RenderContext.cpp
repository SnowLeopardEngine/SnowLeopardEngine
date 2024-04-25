#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    Ref<Shader> RenderContext::CreateGraphicsProgram(const std::string& vertPath,
                                                     const std::string& fragPath,
                                                     const std::optional<std::string>&)
    {
        // TODO: Geometry, Compute
        return Shader::Create(vertPath, fragPath);
    }
} // namespace SnowLeopardEngine