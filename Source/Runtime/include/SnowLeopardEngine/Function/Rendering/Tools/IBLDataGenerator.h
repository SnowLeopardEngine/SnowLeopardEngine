#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/Texture.h"

namespace SnowLeopardEngine
{
    class RenderContext;

    class IBLDataGenerator
    {
    public:
        explicit IBLDataGenerator(RenderContext&);
        ~IBLDataGenerator();

        Texture GenerateBrdfLUT();

        Texture GenerateIrradiance(const Texture& cubemap);
        Texture PrefilterEnvMap(const Texture& cubemap);

    private:
        RenderContext& m_RenderContext;

        GraphicsPipeline m_BrdfPipeline;
        GraphicsPipeline m_IrradiancePipeline;
        GraphicsPipeline m_PrefilterEnvMapPipeline;
    };
} // namespace SnowLeopardEngine