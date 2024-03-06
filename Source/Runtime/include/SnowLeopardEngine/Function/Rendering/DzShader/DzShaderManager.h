#pragma once

#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

namespace SnowLeopardEngine
{
    class Shader;

    class DzShaderManager
    {
    public:
        static void AddShader(const std::string& dzShaderName, const std::filesystem::path& dzShaderFilePath);
        static void AddShader(const DzShader& dzShader);

        static DzShader& GetShader(const std::string& dzShaderName);

        static bool Compile();

        static Ref<Shader> GetCompiledPassShader(const std::string& dzPassName);

        static void SetInt(const std::string& dzShaderName, const std::string& propertyName, int value);

    private:
        static DzShaderCompiler                             s_Compiler;
        static std::unordered_map<std::string, DzShader>    s_DzShaders;
        static std::unordered_map<std::string, Ref<Shader>> s_CompiledShaders;
    };
} // namespace SnowLeopardEngine