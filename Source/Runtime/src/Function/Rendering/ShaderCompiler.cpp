#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Platform/Platform.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

const std::filesystem::path g_ShaderPath = "Assets/Shaders/";

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

    static shaderc_shader_kind ShaderStageToShaderCKind(const std::string& stageSuffix)
    {
        if (stageSuffix == ".vert")
        {
            return shaderc_glsl_vertex_shader;
        }
        else if (stageSuffix == ".frag")
        {
            return shaderc_glsl_fragment_shader;
        }
        else if (stageSuffix == ".geom")
        {
            return shaderc_glsl_geometry_shader;
        }
        else if (stageSuffix == ".comp")
        {
            return shaderc_glsl_compute_shader;
        }
        else
        {
            SNOW_LEOPARD_CORE_ERROR(
                "[ShaderCompiler][ShaderStageToShaderCKind] Unknown or Unsupported stage suffix: {0}", stageSuffix);
            return shaderc_glsl_vertex_shader; // fall back to vertex
        }
    }

    ShaderCompileResult ShaderCompiler::Compile(const std::filesystem::path&    shaderPath,
                                                const std::vector<std::string>& keywords)
    {
        ShaderCompileResult result = {};

        auto glslSource = FileSystem::ReadAllText(shaderPath.generic_string());
        if (glslSource.empty())
        {
            result.Success = false;
            result.Message = fmt::format("Shader file [{0}] not found!", shaderPath.generic_string());

            return result;
        }

        std::vector<uint32_t> spvBinary;
        std::string           message;

        if (!CompileGLSL2SPV(glslSource,
                             shaderPath.extension().generic_string(),
                             "main",
                             shaderPath.filename().generic_string(),
                             keywords,
                             spvBinary,
                             message))
        {
            result.Success = false;
            result.Message = message;

            return result;
        }

        std::string compiledGLSL;
        CompileSPV2GLSL(spvBinary, compiledGLSL);

        result.ProgramCode = compiledGLSL;
        return result;
    }

    bool ShaderCompiler::CompileGLSL2SPV(const std::string&              glslSourceText,
                                         const std::string&              stageSuffix,
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
            compiler.PreprocessGlsl(glslSourceText, ShaderStageToShaderCKind(stageSuffix), shaderName.c_str(), options);
        if (preResult.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            message = preResult.GetErrorMessage();
            return false;
        }

        std::string prePassesString(preResult.begin());

#ifndef NDEBUG
        SNOW_LEOPARD_CORE_INFO("[ShaderCompiler] Preprocessed source:\n{0}", prePassesString);
#endif

        // Compile
        auto compileResult = compiler.CompileGlslToSpv(prePassesString,
                                                       ShaderStageToShaderCKind(stageSuffix),
                                                       shaderName.c_str(),
                                                       stageEntryPoint.c_str(),
                                                       options);
        if (compileResult.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            auto innerErrorMsg = compileResult.GetErrorMessage();
            message = fmt::format("Inner Message: {0}, Preprocessed source: {1}", innerErrorMsg, prePassesString);
            return false;
        }

        spvBinary = std::vector<uint32_t>(compileResult.cbegin(), compileResult.cend());
        return true;
    }

    void ShaderCompiler::CompileSPV2GLSL(const std::vector<uint32_t>& spvBinary, std::string& glslSource)
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

#ifndef NDEBUG
        SNOW_LEOPARD_CORE_INFO("[ShaderCompiler] Compiled source from SPV:\n{0}", glslSource);
#endif
    }
} // namespace SnowLeopardEngine