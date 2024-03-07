#include "SnowLeopardEngine/Platform/OpenGL/GLPipelineStateManager.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"

namespace SnowLeopardEngine
{
    Ref<Shader> GLPipelineStateManager::GetShaderProgram(const Ref<PipelineState>& state)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        auto it = m_ShaderLibrary.find(state->Name);
        SNOW_LEOPARD_CORE_ASSERT(
            it != m_ShaderLibrary.end(), "[GLPipelineStateManager] State {0} not found!", state->Name);

        return it->second;
    }

    bool GLPipelineStateManager::InitState(const Ref<PipelineState>& state)
    {
        if (m_ShaderLibrary.count(state->Name) == 0)
        {
            auto vertexFilePath =
                FileSystem::GetExecutableRelativeDirectory("Assets/Shaders") / state->VertexShaderName;
            auto fragmentFilePath =
                FileSystem::GetExecutableRelativeDirectory("Assets/Shaders") / state->FragmentShaderName;
            auto shader = Shader::Create(FileSystem::ReadAllText(vertexFilePath.generic_string()),
                                         FileSystem::ReadAllText(fragmentFilePath.generic_string()));
            m_ShaderLibrary.emplace(state->Name, shader);
        }

        return true;
    }

    void GLPipelineStateManager::DestroyState(const Ref<PipelineState>& state)
    {
        auto it = m_ShaderLibrary.find(state->Name);
        SNOW_LEOPARD_CORE_ASSERT(
            it != m_ShaderLibrary.end(), "[GLPipelineStateManager] State {0} not found!", state->Name);

        m_ShaderLibrary.erase(it->first);
    }
} // namespace SnowLeopardEngine