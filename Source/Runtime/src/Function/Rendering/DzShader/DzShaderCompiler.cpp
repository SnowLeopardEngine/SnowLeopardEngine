#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Platform/Platform.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

namespace SnowLeopardEngine
{
    // NOLINTBEGIN
    class MyIncluder : public shaderc::CompileOptions::IncluderInterface
    {
    public:
        virtual shaderc_include_result* GetInclude(const char*          requested_source,
                                                   shaderc_include_type type,
                                                   const char*          requesting_source,
                                                   size_t               include_depth) override final
        {
            const std::filesystem::path search_paths[] = {"Assets/Shaders/include"};

            for (const auto& search_path : search_paths)
            {
                std::string   file_path = (search_path / requested_source).generic_string();
                std::ifstream file_stream(file_path.c_str(), std::ios::binary);
                if (file_stream.is_open())
                {
                    FileInfo*         new_file_info = new FileInfo {file_path, {}};
                    std::vector<char> file_content((std::istreambuf_iterator<char>(file_stream)),
                                                   std::istreambuf_iterator<char>());
                    new_file_info->contents = file_content;
                    return new shaderc_include_result {new_file_info->path.data(),
                                                       new_file_info->path.length(),
                                                       new_file_info->contents.data(),
                                                       new_file_info->contents.size(),
                                                       new_file_info};
                }
            }

            return nullptr;
        }

        virtual void ReleaseInclude(shaderc_include_result* data) override final
        {
            FileInfo* info = static_cast<FileInfo*>(data->user_data);
            delete info;
            delete data;
        }

    private:
        struct FileInfo
        {
            const std::string path;
            std::vector<char> contents;
        };

        std::unordered_set<std::string> included_files_;
    };
    // NOLINTEND

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
            SNOW_LEOPARD_CORE_ERROR(
                "[DzShaderCompiler][ShaderStageToShaderCKind] Unknown or Unsupported stage name: {0}", stageName);
            return shaderc_glsl_vertex_shader; // fall back to vertex
        }
    }

    DzShaderCompileResult DzShaderCompiler::Compile(const DzShader& shader)
    {
        DzShaderCompileResult result = {};

        for (const auto& pass : shader.Passes)
        {
            DzShaderCompilePassResult passResult = {};
            passResult.PassName                  = pass.Name;

            for (const auto& stage : pass.Stages)
            {
                // use Google's shaderc to compile source to get SPV
                std::vector<uint32_t> spvBinary;
                std::string           shadercMessage;
                bool                  shadercOK = CompileGLSL2SPV(stage.ShaderSource,
                                                 stage.Name,
                                                 stage.EntryPoint,
                                                 shader.Name,
                                                 shader.Keywords,
                                                 spvBinary,
                                                 shadercMessage);

                if (!shadercOK)
                {
                    result.Success = false;
                    result.Message =
                        std::format("[DzShaderCompiler] Shaderc Error. Failed to compile {0} stage in pass({1}), "
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

    bool DzShaderCompiler::CompileGLSL2SPV(const std::string&              glslSourceText,
                                           const std::string&              stageName,
                                           const std::string&              stageEntryPoint,
                                           const std::string&              shaderName,
                                           const std::vector<std::string>& keywords,
                                           std::vector<uint32_t>&          spvBinary,
                                           std::string&                    message)
    {
        shaderc::Compiler       compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
        // options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetAutoMapLocations(false);
        options.SetAutoBindUniforms(false);
        options.SetIncluder(CreateScope<MyIncluder>());

        // Setup keywords
        for (const auto& keyword : keywords)
        {
            options.AddMacroDefinition(keyword);
        }

        // Preprocess
        auto preResult =
            compiler.PreprocessGlsl(glslSourceText, ShaderStageToShaderCKind(stageName), shaderName.c_str(), options);
        if (preResult.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            message = preResult.GetErrorMessage();
            return false;
        }

        std::string prePassesString(preResult.begin());

        // Compile
        auto compileResult = compiler.CompileGlslToSpv(
            prePassesString, ShaderStageToShaderCKind(stageName), shaderName.c_str(), stageEntryPoint.c_str(), options);
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
        options.version = 410;
        // https://github.com/KhronosGroup/SPIRV-Cross/issues/1981
        options.enable_420pack_extension = false;
#else
        options.version = 330; // fall back
#endif

        spirv_cross::CompilerGLSL glslCompiler(spvBinary);
        glslCompiler.set_common_options(options);

        glslSource = glslCompiler.compile();
    }
} // namespace SnowLeopardEngine