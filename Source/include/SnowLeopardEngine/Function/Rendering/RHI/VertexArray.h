#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/IndexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/VertexBuffer.h"

namespace SnowLeopardEngine
{
    class VertexArray
    {
    public:
        virtual ~VertexArray() = default;

        virtual void Bind() const   = 0;
        virtual void Unbind() const = 0;

        virtual void AddBuffers(const Ref<VertexBuffer>& vertexBuffer, const Ref<IndexBuffer>& indexBuffer) = 0;

        virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const = 0;
        virtual const Ref<IndexBuffer>&               GetIndexBuffer() const   = 0;

        static Ref<VertexArray> Create();
    };
} // namespace SnowLeopardEngine