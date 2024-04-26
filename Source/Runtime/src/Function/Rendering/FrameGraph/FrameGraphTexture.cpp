#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"

namespace SnowLeopardEngine
{
    void FrameGraphTexture2D::create(const Desc& desc, void*)
    {
        m_Texture = Texture2D::Create({desc.Width, desc.Height, desc.Format, desc.Config});
    }

    void FrameGraphTexture2D::destroy(const Desc& desc, void*) {}

    void FrameGraphCubemap::create(const Desc& desc, void*)
    {
        m_Cubemap = Cubemap::Create({desc.Width, desc.Height, desc.Format, desc.Config});
    }

    void FrameGraphCubemap::destroy(const Desc& desc, void*) {}
} // namespace SnowLeopardEngine