#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLFrameBuffer.h"

namespace SnowLeopardEngine
{
    Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferDesc& spec) { return CreateRef<GLFrameBuffer>(spec); }
} // namespace SnowLeopardEngine