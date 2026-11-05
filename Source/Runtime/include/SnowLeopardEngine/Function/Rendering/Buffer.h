#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderHandle.h"

namespace SnowLeopardEngine
{
    class Buffer
    {
        friend class RenderContext;

    public:
        Buffer()              = default;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept;
        virtual ~Buffer();

        Buffer& operator=(const Buffer&) = delete;
        Buffer& operator=(Buffer&&) noexcept;

        explicit operator bool() const;

        int64_t GetSize() const;
        bool    IsMapped() const;

    protected:
        Buffer(RenderHandle id, int64_t size);

        explicit operator RenderHandle() const;

    protected:
        RenderHandle m_Id           = kNullHandle;
        int64_t      m_Size         = 0;
        void*        m_MappedMemory = nullptr;
    };
} // namespace SnowLeopardEngine