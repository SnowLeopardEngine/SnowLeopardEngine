#pragma once

#include "SnowLeopardEngine/PrecompiledHeader.h"

#define SNOW_LEOPARD_NO_COPY_CONSTRUCTOR(className) className(const className&) = delete;
#define SNOW_LEOPARD_NO_MOVE_CONSTRUCTOR(className) className(className&&) = delete;
#define SNOW_LEOPARD_DEFAULT_CONSTRUCTOR(className) className() = default;
#define SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(className) \
    SNOW_LEOPARD_NO_COPY_CONSTRUCTOR(className) SNOW_LEOPARD_NO_MOVE_CONSTRUCTOR(className)
#define SNOW_LEOPARD_DEFAULT_NO_COPY_MOVE_CONSTRUCTOR(className) \
    SNOW_LEOPARD_DEFAULT_CONSTRUCTOR(className) SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(className)

// Custom Assert function
template<typename... Args>
void CustomAssert(bool condition, std::string_view message, Args... args)
{
    if (!condition)
    {
        std::cerr << "Assertion failed: " << std::vformat(message, std::make_format_args(args...)) << std::endl;
        exit(1);
    }
}

#ifdef _DEBUG
#define SNOW_LEOPARD_CORE_ASSERT(...) CustomAssert(__VA_ARGS__)
#else
#define SNOW_LEOPARD_CORE_ASSERT(...)
#endif

#define FLAG_ENUM(name, baseType) \
    static name operator|(name lhs, name rhs) \
    { \
        return static_cast<name>(static_cast<baseType>(lhs) | static_cast<baseType>(rhs)); \
    } \
    static name operator&(name lhs, name rhs) \
    { \
        return static_cast<ClearBit>(static_cast<baseType>(lhs) & static_cast<baseType>(rhs)); \
    }