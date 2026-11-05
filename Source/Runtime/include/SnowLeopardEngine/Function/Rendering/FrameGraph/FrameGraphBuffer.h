#pragma once

#include "SnowLeopardEngine/Function/Rendering/Buffer.h"

namespace SnowLeopardEngine
{
    class FrameGraphBuffer
    {
    public:
        struct Desc
        {
            int64_t Size;
        };

        // NOLINTBEGIN
        void create(const Desc&, void* allocator);
        void destroy(const Desc&, void* allocator);
        // NOLINTEND

        Buffer* Handle = nullptr;
    };
} // namespace SnowLeopardEngine