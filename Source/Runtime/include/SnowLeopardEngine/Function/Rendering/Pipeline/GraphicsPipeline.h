#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"

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

            Builder& SetShaderProgram(RenderHandle program);
            Builder& SetVAO(RenderHandle vao);
            Builder& SetDepthStencil(const DepthStencilState&);
            Builder& SetRasterizerState(const RasterizerState&);
            Builder& SetBlendState(uint32_t attachment, const BlendState&);

            GraphicsPipeline Build();

        private:
            RenderHandle m_Program = kNullHandle;
            RenderHandle m_VAO     = kNullHandle;

            DepthStencilState                          m_DepthStencilState {};
            RasterizerState                            m_RasterizerState {};
            std::array<BlendState, kMaxNumBlendStates> m_BlendStates {};
        };

    private:
        Rect2D m_Viewport, m_Scissor;

        RenderHandle m_Program = kNullHandle;
        RenderHandle m_VAO     = kNullHandle;

        DepthStencilState                          m_DepthStencilState {};
        RasterizerState                            m_RasterizerState {};
        std::array<BlendState, kMaxNumBlendStates> m_BlendStates {};
    };
} // namespace SnowLeopardEngine