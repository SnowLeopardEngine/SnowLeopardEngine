#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/VertexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class GLVertexBuffer : public VertexBuffer
    {
    public:
        explicit GLVertexBuffer(uint32_t size);
        GLVertexBuffer(float* vertices, uint32_t size);
        explicit GLVertexBuffer(const std::vector<MeshVertexData>& vertices);

        virtual ~GLVertexBuffer();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        uint32_t GetName() const { return m_BufferName; }

        virtual void SetBufferData(const void* data, uint32_t size) override;
        virtual void SetBufferData(const std::vector<MeshVertexData>& vertices) override;

        virtual const BufferLayout& GetLayout() const override { return m_Layout; }
        virtual void                SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

    private:
        uint32_t     m_BufferName = 0;
        BufferLayout m_Layout;
    };
} // namespace SnowLeopardEngine