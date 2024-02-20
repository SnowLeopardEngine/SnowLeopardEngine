#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDispatchPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"

namespace SnowLeopardEngine
{
    class RenderPass;

    class Pipeline
    {
    public:
        virtual ~Pipeline() = default;

        virtual bool Init() = 0;
        virtual void Tick(float deltaTime);

        virtual void Shutdown() { ClearStates(); }

    protected:
        bool                      RegisterState(PipelineState& state);
        void                      UnregisterState(PipelineState& state);
        const Ref<PipelineState>& GetState(const std::string& name) const;
        void                      ClearStates();

    protected:
        std::map<std::string, Ref<PipelineState>> m_States;
        std::vector<Ref<RenderDrawPass>>          m_DrawPasses;
        std::vector<Ref<RenderDispatchPass>>      m_DispatchPasses;
    };
} // namespace SnowLeopardEngine