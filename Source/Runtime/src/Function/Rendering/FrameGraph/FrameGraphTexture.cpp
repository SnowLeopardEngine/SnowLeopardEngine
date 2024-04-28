#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"

namespace SnowLeopardEngine
{
    void FrameGraphTexture::create(const Desc& desc, void* allocator)
    {
        Handle = static_cast<TransientResources*>(allocator)->AcquireTexture(desc);
    }

    void FrameGraphTexture::destroy(const Desc& desc, void* allocator)
    {
        static_cast<TransientResources*>(allocator)->ReleaseTexture(desc, Handle);
    }
} // namespace SnowLeopardEngine