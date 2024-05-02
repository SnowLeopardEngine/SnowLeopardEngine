#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"

namespace SnowLeopardEngine
{
    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetShaderProgram(GLuint program)
    {
        m_Program = program;
        return *this;
    }

    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetVAO(GLuint vao)
    {
        m_VAO = vao;
        return *this;
    }

    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetDepthStencil(const DepthStencilState& state)
    {
        m_DepthStencilState = state;
        return *this;
    }

    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetRasterizerState(const RasterizerState& state)
    {
        m_RasterizerState = state;
        return *this;
    }

    GraphicsPipeline::Builder& GraphicsPipeline::Builder::SetBlendState(uint32_t attachment, const BlendState& state)
    {
        assert(attachment < kMaxNumBlendStates);
        m_BlendStates[attachment] = state;
        return *this;
    }

    GraphicsPipeline GraphicsPipeline::Builder::Build()
    {
        GraphicsPipeline g;

        g.m_Program           = m_Program;
        g.m_VAO               = m_VAO;
        g.m_DepthStencilState = m_DepthStencilState;
        g.m_RasterizerState   = m_RasterizerState;
        g.m_BlendStates       = m_BlendStates;

        return g;
    }
} // namespace SnowLeopardEngine