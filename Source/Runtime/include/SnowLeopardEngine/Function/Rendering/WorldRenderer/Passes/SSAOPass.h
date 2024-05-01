#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class SSAOPass
    {
    public:
        explicit SSAOPass(RenderContext&, uint32_t kernelSize = 32u);
        ~SSAOPass();

        void AddToGraph(FrameGraph&, FrameGraphBlackboard&);

    private:
        void GenerateNoiseTexture();
        void GenerateSampleKernel(uint32_t kernelSize);
        void CreatePipeline(uint32_t kernelSize);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;

        Texture       m_Noise;
        UniformBuffer m_KernelBuffer;
    };
} // namespace SnowLeopardEngine