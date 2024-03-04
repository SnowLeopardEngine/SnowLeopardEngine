#include "SnowLeopardEngine/Function/Rendering/RHI/VertexBuffer.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLVertexBuffer.h"

namespace SnowLeopardEngine
{
    Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) { return CreateRef<GLVertexBuffer>(size); }

    Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size)
    {
        return CreateRef<GLVertexBuffer>(vertices, size);
    }

    Ref<VertexBuffer> VertexBuffer::Create(std::vector<StaticMeshVertexData> vertices)
    {
        return CreateRef<GLVertexBuffer>(vertices);
    }

    Ref<VertexBuffer> VertexBuffer::Create(std::vector<AnimatedMeshVertexData> vertices)
    {
        return CreateRef<GLVertexBuffer>(vertices);
    }
} // namespace SnowLeopardEngine