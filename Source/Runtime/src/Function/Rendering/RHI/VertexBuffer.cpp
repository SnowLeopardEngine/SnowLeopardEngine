#include "SnowLeopardEngine/Function/Rendering/RHI/VertexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLVertexBuffer.h"

namespace SnowLeopardEngine
{
    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) { return CreateRef<GLVertexBuffer>(size); }

    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
    {
        return CreateRef<GLVertexBuffer>(vertices, size);
    }

    Ref<VertexBuffer> VertexBuffer::Create(const std::vector<MeshVertexData>& vertices)
    {
        return CreateRef<GLVertexBuffer>(vertices);
    }
} // namespace SnowLeopardEngine