#pragma once

#include "SnowLeopardEngine/PrecompiledHeader.h"

namespace SnowLeopardEngine
{
    template<typename T>
    using Scope = std::unique_ptr<T>;

    template<typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using Ref = std::shared_ptr<T>;

    template<typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    // Trim from both ends (in place)
    static void trim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));

        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    }

    // Split string by comma
    static std::vector<std::string> split(const std::string& s, const char& delim = ',')
    {
        std::vector<std::string> result;
        std::istringstream       iss(s);
        std::string              token;

        while (std::getline(iss, token, delim))
        {
            trim(token);
            result.push_back(token);
        }

        return result;
    }
} // namespace SnowLeopardEngine
