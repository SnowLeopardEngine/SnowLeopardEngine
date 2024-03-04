#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/VertexBuffer.h"

namespace SnowLeopardEngine
{
    class GLVertexBuffer : public VertexBuffer
    {
    public:
        explicit GLVertexBuffer(uint32_t size);
        GLVertexBuffer(float* vertices, uint32_t size);
        explicit GLVertexBuffer(std::vector<StaticMeshVertexData> vertices);
        explicit GLVertexBuffer(std::vector<AnimatedMeshVertexData> vertices);

        virtual ~GLVertexBuffer();

        uint32_t GetName() const { return m_BufferName; }

        virtual void SetBufferData(const void* data, uint32_t size) override;

        virtual const BufferLayout& GetLayout() const override { return m_Layout; }
        virtual void                SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

    private:
        uint32_t     m_BufferName = 0;
        BufferLayout m_Layout;
    };
} // namespace SnowLeopardEngine