#pragma once

// Profiling
#include <tracy/Tracy.hpp>

#include <cstddef>

#define SNOW_LEOPARD_PROFILE_FUNCTION ZoneScoped;
#define SNOW_LEOPARD_PROFILE_NAMED_SCOPE(__VA_ARGS__) ZoneScopedN(__VA_ARGS__);
#define SNOW_LEOPARD_PROFILE_END_OF_FRAME FrameMark;

#ifdef TRACY_ENABLE
namespace SnowLeopardEngine
{
    // A GPU timing zone recorded by the active rendering backend (a Tracy OpenGL zone for now).
    // The backend owns the GPU timer context; callers only see this scope object.
    class GPUProfileZone
    {
    public:
        explicit GPUProfileZone(const tracy::SourceLocationData* location);
        ~GPUProfileZone();

        GPUProfileZone(const GPUProfileZone&)            = delete;
        GPUProfileZone& operator=(const GPUProfileZone&) = delete;

    private:
        // Storage for the backend's scope object, constructed in place by the backend.
        alignas(alignof(std::max_align_t)) unsigned char m_Storage[16];
    };
} // namespace SnowLeopardEngine

#define SNOW_LEOPARD_PROFILE_GPU(name) \
    static constexpr tracy::SourceLocationData TracyConcat(__sle_gpu_source_location, TracyLine) { \
        name, TracyFunction, TracyFile, (uint32_t)TracyLine, 0}; \
    const ::SnowLeopardEngine::GPUProfileZone TracyConcat(__sle_gpu_zone, TracyLine) { \
        &TracyConcat(__sle_gpu_source_location, TracyLine)};
#else
#define SNOW_LEOPARD_PROFILE_GPU(name)
#endif
