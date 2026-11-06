#pragma once

// Currently, we only support Windows, Linux and macOS.

#define SNOW_LEOPARD_PLATFORM_DARWIN 0
#define SNOW_LEOPARD_PLATFORM_LINUX 0
#define SNOW_LEOPARD_PLATFORM_WINDOWS 0

#if defined(__APPLE__) && defined(__MACH__)
#undef SNOW_LEOPARD_PLATFORM_DARWIN
#define SNOW_LEOPARD_PLATFORM_DARWIN 1
#elif defined(__linux__)
#undef SNOW_LEOPARD_PLATFORM_LINUX
#define SNOW_LEOPARD_PLATFORM_LINUX 1
#elif defined(_WIN32) || defined(_WIN64)
#undef SNOW_LEOPARD_PLATFORM_WINDOWS
#define SNOW_LEOPARD_PLATFORM_WINDOWS 1
#else
#error "Unsupported Platform"
#endif

