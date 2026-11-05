#pragma once

#include <cstdint>

namespace SnowLeopardEngine
{
    // Name of a backend object (an OpenGL object name for now). Typed VRI handles replace it in Phase 2.
    using RenderHandle                         = uint32_t;
    inline constexpr RenderHandle kNullHandle = 0;
} // namespace SnowLeopardEngine
