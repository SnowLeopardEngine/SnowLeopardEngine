#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

namespace SnowLeopardEngine
{
    class CubemapConverter
    {
    public:
        explicit CubemapConverter(RenderContext&);
        ~CubemapConverter();

        Texture EquirectangularToCubemap(const Texture&);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine