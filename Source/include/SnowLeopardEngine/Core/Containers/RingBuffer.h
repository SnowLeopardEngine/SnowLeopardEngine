#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    template<typename T>
    class RingBuffer
    {
    public:
        RingBuffer(size_t size) : m_Size(size), m_Head(0), m_Tail(0) { m_Buffer.resize(size); }

        void Push(const T& item)
        {
            m_Buffer[m_Head] = item;
            m_Head           = (m_Head + 1) % m_Size;
            if (m_Head == m_Tail)
            {
                m_Tail = (m_Tail + 1) % m_Size;
            }
        }

        T& Front() { return m_Buffer[m_Tail]; }

        void Pop()
        {
            if (m_Head != m_Tail)
            {
                m_Tail = (m_Tail + 1) % m_Size;
            }
        }

        size_t GetSize() const { return m_Size; }

        size_t GetCount() const { return (m_Head - m_Tail + m_Size) % m_Size; }

    private:
        std::vector<T> m_Buffer;
        size_t         m_Size;
        size_t         m_Head;
        size_t         m_Tail;
    };
} // namespace SnowLeopardEngine
