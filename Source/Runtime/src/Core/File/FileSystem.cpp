#include "SnowLeopardEngine/Core/File/FileSystem.h"

#include <cryptopp/hex.h>
#include <cryptopp/md5.h>

namespace SnowLeopardEngine
{
    std::filesystem::path FileSystem::s_ExeDirectory;

    void FileSystem::InitExecutableDirectory(const char* executableFilePath)
    {
        std::filesystem::path exePath(executableFilePath);
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
        CryptoPP::Weak1::MD5 md5;
        CryptoPP::byte       digest[CryptoPP::Weak1::MD5::DIGESTSIZE];

        std::ifstream file(path.generic_string(), std::ios::binary);
        if (!file.is_open())
        {
            return std::string();
        }

        while (file)
        {
            char buffer[1024];
            file.read(buffer, sizeof(buffer));
            md5.Update(reinterpret_cast<const CryptoPP::byte*>(buffer), file.gcount());
        }

        md5.Final(digest);

        CryptoPP::HexEncoder encoder;
        std::string          result;
        encoder.Attach(new CryptoPP::StringSink(result));
        encoder.Put(digest, sizeof(digest));
        encoder.MessageEnd();

        return result;
    }
} // namespace SnowLeopardEngine