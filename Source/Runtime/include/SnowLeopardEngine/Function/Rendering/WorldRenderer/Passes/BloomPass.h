#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DownsamplePass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/UpsamplePass.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class BloomPass
    {
    public:
        explicit BloomPass(RenderContext&);
        ~BloomPass();

        FrameGraphResource Resample(FrameGraph&, FrameGraphResource input, float radius);
        FrameGraphResource Compose(FrameGraph&, FrameGraphResource scene, FrameGraphResource bloom, float strength);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;

        UpsamplePass   m_Upsampler;
        DownsamplePass m_Downsampler;
    };
} // namespace SnowLeopardEngine