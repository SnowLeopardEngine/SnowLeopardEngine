#pragma once

#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

namespace SnowLeopardEngine
{
    class Shader;

    class DzShaderManager
    {
    public:
        static void AddShader(const std::filesystem::path& dzShaderFilePath);
        static void AddShader(const DzShader& dzShader);

        static bool Compile();

        static Ref<Shader> GetCompiledPassShader(const std::string& dzPassName);

    private:
        static DzShaderCompiler                             s_Compiler;
        static std::vector<DzShader>                        s_DzShaders;
        static std::unordered_map<std::string, Ref<Shader>> s_CompiledShaders;
    };
} // namespace SnowLeopardEngine