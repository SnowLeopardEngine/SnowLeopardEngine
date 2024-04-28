#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    class GraphicsPipeline
    {
    public:
        friend class RenderContext;
        GraphicsPipeline() = default;

        class Builder
        {
        public:
            Builder() = default;

            Builder& SetShaderProgram(GLuint program);
            Builder& SetVAO(GLuint vao);
            Builder& SetDepthStencil(const DepthStencilState&);
            Builder& SetRasterizerState(const RasterizerState&);
            Builder& SetBlendState(uint32_t attachment, const BlendState&);

            GraphicsPipeline Build();

        private:
            GLuint m_Program = GL_NONE;
            GLuint m_VAO     = GL_NONE;

            DepthStencilState                          m_DepthStencilState {};
            RasterizerState                            m_RasterizerState {};
            std::array<BlendState, kMaxNumBlendStates> m_BlendStates {};
        };

    private:
        Rect2D m_Viewport, m_Scissor;

        GLuint m_Program = GL_NONE;
        GLuint m_VAO     = GL_NONE;

        DepthStencilState                          m_DepthStencilState {};
        RasterizerState                            m_RasterizerState {};
        std::array<BlendState, kMaxNumBlendStates> m_BlendStates {};
    };
} // namespace SnowLeopardEngine