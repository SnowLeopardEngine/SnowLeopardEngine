#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"

namespace SnowLeopardEngine
{
    struct Buffer
    {
        void*    Data;
        uint64_t Size;

        Buffer() : Data(nullptr), Size(0) {}

        explicit Buffer(const void* data, uint64_t size = 0) : Data(const_cast<void*>(data)), Size(size) {}

        static Buffer Copy(const Buffer& other)
        {
            Buffer buffer;
            buffer.Allocate(other.Size);
            memcpy(buffer.Data, other.Data, other.Size);
            return buffer;
        }

        static Buffer Copy(const void* data, uint64_t size)
        {
            Buffer buffer;
            buffer.Allocate(size);
            memcpy(buffer.Data, data, size);
            return buffer;
        }

        void Allocate(uint64_t size)
        {
            delete[] static_cast<std::byte*>(Data);
            Data = nullptr;

            if (size == 0)
                return;

            Data = new std::byte[size];
            Size = size;
        }

        void Release()
        {
            delete[] static_cast<std::byte*>(Data);
            Data = nullptr;
            Size = 0;
        }

        void ZeroInitialize() const
        {
            if (Data)
                memset(Data, 0, Size);
        }

        template<typename T>
        T& Read(uint64_t offset = 0)
        {
            return *static_cast<T*>(static_cast<std::byte*>(Data) + offset);
        }

        template<typename T>
        const T& Read(uint64_t offset = 0) const
        {
            return *static_cast<T*>(static_cast<std::byte*>(Data) + offset);
        }

        std::byte* ReadBytes(uint64_t size, uint64_t offset) const
        {
            SNOW_LEOPARD_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
            std::byte* buffer = new std::byte[size];
            memcpy(buffer, static_cast<std::byte*>(Data) + offset, size);
            return buffer;
        }

        void Write(const void* data, uint64_t size, uint64_t offset = 0) const
        {
            SNOW_LEOPARD_CORE_ASSERT(offset + size <= Size, "Buffer overflow!");
            memcpy(static_cast<std::byte*>(Data) + offset, data, size);
        }

        explicit operator bool() const { return Data; }

        std::byte& operator[](int index) const { return (static_cast<std::byte*>(Data))[index]; }

        template<typename T>
        T* As() const
        {
            return static_cast<T*>(Data);
        }

        inline uint64_t GetSize() const { return Size; }
    };

    struct BufferSafe : public Buffer
    {
        ~BufferSafe() { Release(); }

        static BufferSafe Copy(const void* data, uint64_t size)
        {
            BufferSafe buffer;
            buffer.Allocate(size);
            memcpy(buffer.Data, data, size);
            return buffer;
        }
    };
} // namespace SnowLeopardEngine