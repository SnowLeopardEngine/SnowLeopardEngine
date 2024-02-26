#pragma once

namespace SnowLeopardEngine
{
    class Pipeline;

    class RenderPass
    {
    public:
        virtual ~RenderPass() = default;

        virtual void BeginPass() = 0;
        virtual void EndPass()   = 0;

        void      BindPipeline(Pipeline* pipeline) { m_Pipeline = pipeline; }
        Pipeline* GetPipeline() { return m_Pipeline; }

    protected:
        Pipeline* m_Pipeline = nullptr;
    };
} // namespace SnowLeopardEngine