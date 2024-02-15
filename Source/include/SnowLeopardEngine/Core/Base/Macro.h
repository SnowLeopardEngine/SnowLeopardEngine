#pragma once

#include "SnowLeopardEngine/PrecompiledHeader.h"

#include <fmt/core.h>

#define SNOW_LEOPARD_NO_COPY_CONSTRUCTOR(className) className(const className&) = delete;
#define SNOW_LEOPARD_NO_MOVE_CONSTRUCTOR(className) className(className&&) = delete;
#define SNOW_LEOPARD_DEFAULT_CONSTRUCTOR(className) className() = default;
#define SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(className) \
    SNOW_LEOPARD_NO_COPY_CONSTRUCTOR(className) SNOW_LEOPARD_NO_MOVE_CONSTRUCTOR(className)
#define SNOW_LEOPARD_DEFAULT_NO_COPY_MOVE_CONSTRUCTOR(className) \
    SNOW_LEOPARD_DEFAULT_CONSTRUCTOR(className) SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(className)

// Custom Assert function
template<typename... Args>
void CustomAssert(bool condition, const char* message, Args... args)
{
    if (!condition)
    {
        std::cerr << "Assertion failed: " << fmt::format(message, args...) << std::endl;
        exit(1);
    }
}

#define SNOW_LEOPARD_CORE_ASSERT(cond, msg, ...) CustomAssert(cond, msg, __VA_ARGS__)