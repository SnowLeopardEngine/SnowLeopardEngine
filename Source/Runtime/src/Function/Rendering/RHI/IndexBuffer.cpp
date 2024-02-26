#include "SnowLeopardEngine/Function/Rendering/RHI/IndexBuffer.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLIndexBuffer.h"

namespace SnowLeopardEngine
{
    Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
    {
        return CreateRef<GLIndexBuffer>(indices, count);
    }

    Ref<IndexBuffer> IndexBuffer::Create(std::vector<uint32_t> indices) { return CreateRef<GLIndexBuffer>(indices); }
} // namespace SnowLeopardEngine