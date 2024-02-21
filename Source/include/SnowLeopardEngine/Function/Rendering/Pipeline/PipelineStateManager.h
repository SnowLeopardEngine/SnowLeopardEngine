#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    class PipelineStateManager
    {
    public:
        virtual ~PipelineStateManager() = default;

        bool                      RegisterState(const Ref<PipelineState>& state);
        void                      UnregisterState(const Ref<PipelineState>& state);
        const Ref<PipelineState>& GetState(const std::string& name) const;
        void                      ClearStates();

        virtual Ref<Shader> GetShaderProgram(const Ref<PipelineState>& state) = 0;

        static Ref<PipelineStateManager> Create();

    protected:
        virtual bool InitState(const Ref<PipelineState>& state)    = 0;
        virtual void DestroyState(const Ref<PipelineState>& state) = 0;

    protected:
        std::map<std::string, Ref<PipelineState>> m_States;
    };
} // namespace SnowLeopardEngine