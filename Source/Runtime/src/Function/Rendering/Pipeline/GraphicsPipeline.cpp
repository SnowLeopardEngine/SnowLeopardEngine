#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/VertexArray.h"

namespace SnowLeopardEngine
{
    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetShaderProgram(const Ref<Shader>& program)
    {
        m_Program = program;
        return *this;
    }

    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetVertexArray(const Ref<VertexArray>& vao)
    {
        m_VAO = vao;
        return *this;
    }

    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetPipelineState(const PipelineState& state)
    {
        m_State = state;
        return *this;
    }

    GraphicsPipeline GraphicsPipeline::Builder::Build()
    {
        GraphicsPipeline g;

        g.m_Program = m_Program;
        g.m_VAO     = m_VAO;
        g.m_State   = m_State;

        return g;
    }
} // namespace SnowLeopardEngine