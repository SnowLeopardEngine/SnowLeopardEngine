#pragma once

#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

namespace SnowLeopardEngine
{
    class Shader;
    class FrameBuffer;

    class DzShaderManager
    {
    public:
        static void AddShader(const std::string& dzShaderName, const std::filesystem::path& dzShaderFilePath);
        static void AddShader(const DzShader& dzShader);

        static DzShader& GetShader(const std::string& dzShaderName);

        static bool Compile();

        static void BindPassResources(const DzPass& dzPass);
        static void UnbindPassResources(const DzPass& dzPass);

        static void UsePassResources(const DzPass& dzPass, const Ref<Shader>& shader, int& resourceBinding);

        static Ref<Shader> GetCompiledPassShader(const std::string& dzPassName);

    private:
        static DzShaderCompiler                             s_Compiler;
        static std::unordered_map<std::string, DzShader>    s_DzShaders;
        static std::unordered_map<std::string, Ref<Shader>> s_CompiledShaders;
        static std::unordered_map<std::string, DzResource>  s_RenderResources;
    };
} // namespace SnowLeopardEngine