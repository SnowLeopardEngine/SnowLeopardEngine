#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    DzShaderCompiler                             DzShaderManager::s_Compiler;
    std::vector<DzShader>                        DzShaderManager::s_DzShaders;
    std::unordered_map<std::string, Ref<Shader>> DzShaderManager::s_CompiledShaders;

    void DzShaderManager::AddShader(const std::filesystem::path& dzShaderFilePath)
    {
        std::ifstream            shaderFile(dzShaderFilePath);
        cereal::JSONInputArchive archive(shaderFile);

        DzShader shaderFromFile;
        archive(shaderFromFile);

        AddShader(shaderFromFile);
    }

    void DzShaderManager::AddShader(const DzShader& dzShader) { s_DzShaders.emplace_back(dzShader); }

    bool DzShaderManager::Compile()
    {
        bool hasError = false;

        for (const auto& dzShader : s_DzShaders)
        {
            auto compileResult = s_Compiler.Compile(dzShader);
            if (!compileResult.Success)
            {
                SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] Failed to compile DzShader {0}, Message: {1}",
                                        dzShader.Name,
                                        compileResult.Message);
                hasError = true;
            }

            for (const auto& passResult : compileResult.PassResults)
            {
                s_CompiledShaders[passResult.PassName] = Shader::Create(passResult.ShaderStageSources);
            }
        }

        return !hasError;
    }

    Ref<Shader> DzShaderManager::GetCompiledPassShader(const std::string& dzPassName)
    {
        if (s_CompiledShaders.count(dzPassName) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] DzPass {0} not found or not compiled!", dzPassName);
            return nullptr;
        }

        return s_CompiledShaders[dzPassName];
    }
} // namespace SnowLeopardEngine