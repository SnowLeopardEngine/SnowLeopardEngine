#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"

namespace SnowLeopardEngine
{
    void FrameGraphBuffer::create(const Desc& desc, void* allocator)
    {
        Handle = static_cast<TransientResources*>(allocator)->AcquireBuffer(desc);
    }

    void FrameGraphBuffer::destroy(const Desc& desc, void* allocator)
    {
        static_cast<TransientResources*>(allocator)->ReleaseBuffer(desc, Handle);
    }
} // namespace SnowLeopardEngine