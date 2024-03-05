#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Platform/Platform.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace SnowLeopardEngine
{
    static shaderc_shader_kind ShaderStageToShaderCKind(const std::string& stageName)
    {
        if (stageName == "vertex")
        {
            return shaderc_glsl_vertex_shader;
        }
        else if (stageName == "fragment")
        {
            return shaderc_glsl_fragment_shader;
        }
        else if (stageName == "geometry")
        {
            return shaderc_glsl_geometry_shader;
        }
        else if (stageName == "compute")
        {
            return shaderc_glsl_compute_shader;
        }
        else
        {
            SNOW_LEOPARD_CORE_ERROR("[DzShaderCompiler][ShaderStageToShaderCKind] Unknown stage name: {0}", stageName);
            return shaderc_glsl_vertex_shader; // fall back to vertex
        }
    }

    DzShaderCompileResult DzShaderCompiler::Compile(const DzShader& shader)
    {
        DzShaderCompileResult result = {};

        for (const auto& pass : shader.Passes)
        {
            DzShaderCompilePassResult passResult = {};

            for (const auto& stage : pass.Stages)
            {
                // use Google's shaderc to compile source to get SPV
                std::vector<uint32_t> spvBinary;
                std::string           shadercMessage;
                bool shadercOK = CompileGLSL2SPV(stage.ShaderSource, stage.Name, shader.Name, spvBinary, shadercMessage);

                if (!shadercOK)
                {
                    result.Success = false;
                    result.Message =
                        fmt::format("[DzShaderCompiler] Shaderc Error. Failed to compile {0} stage in pass({1}), "
                                    "message from shaderc: {2}",
                                    stage.Name,
                                    pass.Name,
                                    shadercMessage);

                    return result;
                }

                // use SPIRV-Cross to compile SPV to GLSL
                std::string glslSource;
                std::string spirvCrossMessage;
                CompileSPV2GLSL(spvBinary, glslSource);

                passResult.ShaderStageSources[stage.Name] = glslSource;
            }

            result.PassResults.emplace_back(passResult);
        }

        return result;
    }

    bool DzShaderCompiler::CompileGLSL2SPV(const std::string&     glslSourceText,
                                           const std::string&     stageName,
                                           const std::string&     shaderName,
                                           std::vector<uint32_t>& spvBinary,
                                           std::string&           message)
    {
        shaderc::Compiler       compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        // options.SetOptimizationLevel(shaderc_optimization_level_performance);

        auto compileResult =
            compiler.CompileGlslToSpv(glslSourceText, ShaderStageToShaderCKind(stageName), shaderName.c_str(), options);
        if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            message = compileResult.GetErrorMessage();
            return false;
        }

        spvBinary = std::vector<uint32_t>(compileResult.cbegin(), compileResult.cend());
        return true;
    }

    void DzShaderCompiler::CompileSPV2GLSL(const std::vector<uint32_t>& spvBinary, std::string& glslSource)
    {
        spirv_cross::CompilerGLSL::Options options;
        options.es = false;
#if SNOW_LEOPARD_PLATFORM_WINDOWS || SNOW_LEOPARD_PLATFORM_LINUX
        options.version = 450;
#elif SNOW_LEOPARD_PLATFORM_DARWIN
        options.version = 410; // MacOS
#else
        options.version = 330; // fall back
#endif

        spirv_cross::CompilerGLSL glslCompiler(spvBinary);
        glslCompiler.set_common_options(options);

        glslSource = glslCompiler.compile();
    }
} // namespace SnowLeopardEngine