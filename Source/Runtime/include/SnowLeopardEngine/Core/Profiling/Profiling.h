#pragma once

// Profiling
#include <tracy/Tracy.hpp>

#define SNOW_LEOPARD_PROFILE_FUNCTION ZoneScoped;
#define SNOW_LEOPARD_PROFILE_NAMED_SCOPE(__VA_ARGS__) ZoneScopedN(__VA_ARGS__);
#define SNOW_LEOPARD_PROFILE_END_OF_FRAME FrameMark;

#include <glad/glad.h>
#include <tracy/TracyOpenGL.hpp>

#define SNOW_LEOPARD_PROFILE_GL_INIT_CONTEXT TracyGpuContext;
#define SNOW_LEOPARD_PROFILE_GL(__VA_ARGS__) TracyGpuZone(__VA_ARGS__);
#define SNOW_LEOPARD_PROFILE_GL_COLLECT TracyGpuCollect;