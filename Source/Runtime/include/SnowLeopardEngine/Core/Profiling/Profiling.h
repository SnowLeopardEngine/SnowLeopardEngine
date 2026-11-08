#pragma once

// Profiling
#include <tracy/Tracy.hpp>

#define SNOW_LEOPARD_PROFILE_FUNCTION ZoneScoped;
#define SNOW_LEOPARD_PROFILE_NAMED_SCOPE(__VA_ARGS__) ZoneScopedN(__VA_ARGS__);
#define SNOW_LEOPARD_PROFILE_END_OF_FRAME FrameMark;

// GPU zones: every render-graph pass is timed with VRI timestamp queries (RHI::GpuProfiler) and appears in Tracy's
// "VRI" GPU context; RenderSystem::GetPassTimings has the numbers. No per-call-site GPU macro is needed.
