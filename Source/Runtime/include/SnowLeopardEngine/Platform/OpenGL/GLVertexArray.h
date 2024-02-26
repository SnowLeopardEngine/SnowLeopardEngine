#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/VertexArray.h"

namespace SnowLeopardEngine
{
    class GLVertexArray : public VertexArray
    {
    public:
        GLVertexArray();
        virtual ~GLVertexArray();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void AddBuffers(const Ref<VertexBuffer>& vertexBuffer, const Ref<IndexBuffer>& indexBuffer) override;

        virtual const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const override { return m_VertexBuffers; }
        virtual const Ref<IndexBuffer>&               GetIndexBuffer() const override { return m_IndexBuffer; }

    private:
        uint32_t                       m_VertexArrayName   = 0;
        uint32_t                       m_VertexBufferIndex = 0;
        std::vector<Ref<VertexBuffer>> m_VertexBuffers;
        Ref<IndexBuffer>               m_IndexBuffer;
    };
} // namespace SnowLeopardEngine