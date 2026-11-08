#include "SnowLeopardEngine/Core/File/FileSystem.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>

namespace SnowLeopardEngine
{
    namespace
    {
        // MD5 (RFC 1321). Only content hashes of asset files (change detection in the asset cache), so a small
        // implementation here instead of a crypto library (cryptopp was the only user, and broke with VS 2026).
        class MD5
        {
        public:
            void Update(const uint8_t* data, size_t size)
            {
                m_Length += size;
                while (size > 0)
                {
                    const size_t take = std::min(size, 64 - m_Buffered);
                    std::memcpy(m_Block + m_Buffered, data, take);
                    m_Buffered += take;
                    data += take;
                    size -= take;
                    if (m_Buffered == 64)
                    {
                        Transform(m_Block);
                        m_Buffered = 0;
                    }
                }
            }

            std::array<uint8_t, 16> Final()
            {
                const uint64_t bits = m_Length * 8;
                const uint8_t  one  = 0x80, zero = 0;
                Update(&one, 1);
                while (m_Buffered != 56)
                {
                    Update(&zero, 1);
                }
                uint8_t length[8];
                for (int i = 0; i < 8; ++i)
                {
                    length[i] = uint8_t(bits >> (8 * i));
                }
                Update(length, 8);
                std::array<uint8_t, 16> digest {};
                for (int i = 0; i < 16; ++i)
                {
                    digest[i] = uint8_t(m_State[i / 4] >> (8 * (i % 4)));
                }
                return digest;
            }

        private:
            static uint32_t Rotate(uint32_t x, int c) { return (x << c) | (x >> (32 - c)); }

            void Transform(const uint8_t* block)
            {
                static constexpr int kShift[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                                                   5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                                                   4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                                                   6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
                static constexpr uint32_t kTable[64] = {
                    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
                    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
                    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
                    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
                    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
                    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
                    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
                    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
                uint32_t m[16];
                for (int i = 0; i < 16; ++i)
                {
                    m[i] = uint32_t(block[i * 4]) | (uint32_t(block[i * 4 + 1]) << 8) | (uint32_t(block[i * 4 + 2]) << 16) |
                           (uint32_t(block[i * 4 + 3]) << 24);
                }
                uint32_t a = m_State[0], b = m_State[1], c = m_State[2], d = m_State[3];
                for (int i = 0; i < 64; ++i)
                {
                    uint32_t f;
                    int      g;
                    if (i < 16)
                    {
                        f = (b & c) | (~b & d);
                        g = i;
                    }
                    else if (i < 32)
                    {
                        f = (d & b) | (~d & c);
                        g = (5 * i + 1) % 16;
                    }
                    else if (i < 48)
                    {
                        f = b ^ c ^ d;
                        g = (3 * i + 5) % 16;
                    }
                    else
                    {
                        f = c ^ (b | ~d);
                        g = (7 * i) % 16;
                    }
                    const uint32_t next = d;
                    d                   = c;
                    c                   = b;
                    b                   = b + Rotate(a + f + kTable[i] + m[g], kShift[i]);
                    a                   = next;
                }
                m_State[0] += a;
                m_State[1] += b;
                m_State[2] += c;
                m_State[3] += d;
            }

            uint32_t m_State[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
            uint8_t  m_Block[64] {};
            size_t   m_Buffered = 0;
            uint64_t m_Length   = 0;
        };
    } // namespace

    std::filesystem::path FileSystem::s_ExeDirectory;

    void FileSystem::InitExecutableDirectory(const char* executableFilePath)
    {
        // argv[0] may be a bare file name (e.g. when launched by a shell from the exe's folder); a relative path would
        // make the directory empty, so resolve it against the working directory first.
        std::error_code       error;
        std::filesystem::path exePath = std::filesystem::absolute(executableFilePath, error);
        if (error)
        {
            exePath = executableFilePath;
        }
        s_ExeDirectory = exePath.parent_path();
    }

    const std::filesystem::path& FileSystem::GetExecutableDirectory() { return s_ExeDirectory; }

    std::filesystem::path FileSystem::GetExecutableRelativeDirectory(const std::filesystem::path& path)
    {
        return s_ExeDirectory / path;
    }

    bool FileSystem::Exists(const std::string& path) { return std::filesystem::exists(path); }

    bool FileSystem::Exists(const std::filesystem::path& path) { return std::filesystem::exists(path); }

    std::string FileSystem::GetFileName(const std::string& filePath)
    {
        return std::filesystem::path(filePath).filename().string();
    }

    bool FileSystem::WriteAllText(const std::string& path, const std::string& content)
    {
        std::ofstream out(path);
        if (out.bad())
        {
            return false;
        }

        out << content;

        out.close();
        return true;
    }

    std::string FileSystem::ReadAllText(const std::string& path)
    {
        std::ifstream in(path);

        if (in.bad())
        {
            return std::string();
        }

        std::stringstream ss;
        ss << in.rdbuf();

        return static_cast<std::string>(ss.str());
    }

    std::string FileSystem::GetMD5(const std::filesystem::path& path)
    {
        MD5 md5;

        std::ifstream file(path.generic_string(), std::ios::binary);
        if (!file.is_open())
        {
            return std::string();
        }

        while (file)
        {
            char buffer[1024];
            file.read(buffer, sizeof(buffer));
            md5.Update(reinterpret_cast<const uint8_t*>(buffer), size_t(file.gcount()));
        }

        // Upper-case hex, like the hashes already stored in project files.
        static constexpr char kHex[] = "0123456789ABCDEF";
        std::string           result;
        for (const auto byte : md5.Final())
        {
            result += kHex[byte >> 4];
            result += kHex[byte & 15];
        }
        return result;
    }
} // namespace SnowLeopardEngine