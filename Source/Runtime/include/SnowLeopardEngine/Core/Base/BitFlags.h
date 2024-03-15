#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    template<typename T>
    class BitFlags
    {
        static_assert(std::is_integral<T>::value, "BitFlags<T>: T must be an integral type");

    public:
        explicit BitFlags(T value = 0) : m_Value(value) {}

        void SetFlag(T flag) { m_Value |= flag; }
        void ClearFlag(T flag) { m_Value &= ~flag; }
        bool HasFlag(T flag) const { return (m_Value & flag) == flag; }

    private:
        T m_Value;
    };

#define DEFINE_BITFLAGS(flagName, baseType) \
    template<> \
    class BitFlags<flagName> \
    { \
    public: \
        explicit BitFlags(flagName value) : m_Value(value) {} \
\
        void SetFlag(flagName flag) \
        { \
            m_Value = static_cast<flagName>(static_cast<baseType>(m_Value) | static_cast<baseType>(flag)); \
        } \
        void ClearFlag(flagName flag) \
        { \
            m_Value = static_cast<flagName>(static_cast<baseType>(m_Value) & ~static_cast<baseType>(flag)); \
        } \
        bool HasFlag(flagName flag) const \
        { \
            return (static_cast<baseType>(m_Value) & static_cast<baseType>(flag)) == static_cast<baseType>(flag); \
        } \
\
    private: \
        flagName m_Value; \
    };
} // namespace SnowLeopardEngine