#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class FileSystem
    {
    public:
        static void                         InitExecutableDirectory(const char* executableFilePath);
        static const std::filesystem::path& GetExecutableDirectory();
        static std::filesystem::path        GetExecutableRelativeDirectory(const std::filesystem::path& path);
        static bool                         Exists(const std::string& path);
        static bool                         Exists(const std::filesystem::path& path);
        static std::string                  GetFileName(const std::string& filePath);
        static bool                         WriteAllText(const std::string& path, const std::string& content);
        static std::string                  ReadAllText(const std::string& path);

    private:
        static std::filesystem::path s_ExeDirectory;
    };
} // namespace SnowLeopardEngine