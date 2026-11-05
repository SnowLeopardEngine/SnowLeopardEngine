// OpenGL backend: Tracy GPU zones behind the backend-neutral GPUProfileZone.
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"

#ifdef TRACY_ENABLE
// clang-format off
#include <glad/glad.h>
#include <tracy/TracyOpenGL.hpp>
// clang-format on

#include <new>

namespace SnowLeopardEngine
{
    using GLZone = tracy::GpuCtxScope;

    GPUProfileZone::GPUProfileZone(const tracy::SourceLocationData* location)
    {
        static_assert(sizeof(GLZone) <= sizeof(m_Storage) && alignof(GLZone) <= alignof(std::max_align_t),
                      "GPUProfileZone storage is too small for tracy::GpuCtxScope");
        new (m_Storage) GLZone(location, true);
    }

    GPUProfileZone::~GPUProfileZone() { std::launder(reinterpret_cast<GLZone*>(m_Storage))->~GLZone(); }
} // namespace SnowLeopardEngine
#endif
