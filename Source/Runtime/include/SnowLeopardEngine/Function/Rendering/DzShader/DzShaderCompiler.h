#pragma once

#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

namespace SnowLeopardEngine
{
    struct DzShaderCompilePassResult
    {
        // key: stage name; value: shader source (text)
        std::unordered_map<std::string, std::string> ShaderStageSources;

        // bool              IsSPV     = false; // enable if switch to Vulkan

        // key: stage name; value: SPIR-V binary
        // std::unordered_map<std::string, std::vector<uint32_t>> ShaderStageSPVs; // enable if switch to Vulkan
    };

    struct DzShaderCompileResult
    {
        bool        Success = true;
        std::string Message;

        std::vector<DzShaderCompilePassResult> PassResults;
    };

    class DzShaderCompiler
    {
    public:
        static DzShaderCompileResult Compile(const DzShader& shader);

    private:
        static bool CompileGLSL2SPV(const std::string&     glslSourceText,
                                    const std::string&     stageName,
                                    const std::string&     shaderName,
                                    std::vector<uint32_t>& spvBinary,
                                    std::string&           message);

        static void CompileSPV2GLSL(const std::vector<uint32_t>& spvBinary, std::string& glslSource);
    };
} // namespace SnowLeopardEngine