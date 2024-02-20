#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class VertexBuffer
    {
    public:
        virtual ~VertexBuffer() = default;

        virtual void SetBufferData(const void* data, uint32_t size) = 0;

        static Ref<VertexBuffer> Create(uint32_t size);
        static Ref<VertexBuffer> Create(float* vertices, uint32_t size);
        static Ref<VertexBuffer> Create(std::vector<VertexData> vertices);
    };
} // namespace SnowLeopardEngine