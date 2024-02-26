#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineStateManager.h"
#include <unordered_map>

namespace SnowLeopardEngine
{
    class GLPipelineStateManager final : public PipelineStateManager
    {
    public:
        virtual Ref<Shader> GetShaderProgram(const Ref<PipelineState>& state) override final;

    protected:
        virtual bool InitState(const Ref<PipelineState>& state) override final;
        virtual void DestroyState(const Ref<PipelineState>& state) override final;

    private:
        std::unordered_map<std::string, Ref<Shader>> m_ShaderLibrary;
    };
} // namespace SnowLeopardEngine