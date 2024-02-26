#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLShader.h"

namespace SnowLeopardEngine
{
    Ref<Shader> Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
    {
        return CreateRef<GLShader>(vertexSource, fragmentSource);
    }
} // namespace SnowLeopardEngine