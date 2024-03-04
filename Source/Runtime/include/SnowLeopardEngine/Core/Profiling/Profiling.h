#pragma once

// Profiling
#include <tracy/Tracy.hpp>
#define SNOW_LEOPARD_PROFILE_FUNCTION ZoneScoped;
#define SNOW_LEOPARD_PROFILE_END_OF_FRAME FrameMark;