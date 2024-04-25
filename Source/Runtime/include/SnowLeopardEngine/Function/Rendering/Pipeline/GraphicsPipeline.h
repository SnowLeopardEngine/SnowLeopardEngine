#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"

namespace SnowLeopardEngine
{
    class Shader;
    class VertexArray;

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline() = default;

        class Builder
        {
        public:
            Builder() = default;

            Builder& SetShaderProgram(const Ref<Shader>& program);
            Builder& SetVertexArray(const Ref<VertexArray>& vao);

            Builder& SetPipelineState(const PipelineState& state);

            GraphicsPipeline Build();

        private:
            Ref<Shader>      m_Program = nullptr;
            Ref<VertexArray> m_VAO     = nullptr;
            PipelineState    m_State;
        };

    private:
        Ref<Shader>      m_Program = nullptr;
        Ref<VertexArray> m_VAO     = nullptr;
        PipelineState    m_State;
    };
} // namespace SnowLeopardEngine