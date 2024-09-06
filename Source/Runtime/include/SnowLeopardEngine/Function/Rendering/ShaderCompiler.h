#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    struct ShaderCompileResult
    {
        bool        Success = true;
        std::string Message;

        std::string ProgramCode;
    };

    class ShaderCompiler
    {
    public:
        static ShaderCompileResult
        Compile(const std::filesystem::path&                                                        shaderPath,
                const std::vector<std::variant<std::string, std::tuple<std::string, std::string>>>& keywords = {});

    private:
        static bool CompileGLSL2SPV(const std::string& glslSourceText,
                                    const std::string& stageSuffix,
                                    const std::string& stageEntryPoint,
                                    const std::string& shaderName,
                                    const std::vector<std::variant<std::string, std::tuple<std::string, std::string>>>&,
                                    std::vector<uint32_t>& spvBinary,
                                    std::string&           message);

        static void CompileSPV2GLSL(const std::vector<uint32_t>& spvBinary, std::string& glslSource);
    };
} // namespace SnowLeopardEngine