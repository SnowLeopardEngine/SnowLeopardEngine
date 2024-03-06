#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    DzShaderCompiler                             DzShaderManager::s_Compiler;
    std::unordered_map<std::string, DzShader>    DzShaderManager::s_DzShaders;
    std::unordered_map<std::string, Ref<Shader>> DzShaderManager::s_CompiledShaders;

    void DzShaderManager::AddShader(const std::string& dzShaderName, const std::filesystem::path& dzShaderFilePath)
    {
        if (s_DzShaders.count(dzShaderName) > 0)
        {
            return;
        }
        
        std::ifstream            shaderFile(dzShaderFilePath);
        cereal::JSONInputArchive archive(shaderFile);

        DzShader shaderFromFile;
        archive(shaderFromFile);

        AddShader(shaderFromFile);
    }

    void DzShaderManager::AddShader(const DzShader& dzShader) { s_DzShaders[dzShader.Name] = dzShader; }

    DzShader& DzShaderManager::GetShader(const std::string& dzShaderName)
    {
        SNOW_LEOPARD_CORE_ASSERT(s_DzShaders.count(dzShaderName) > 0,
                                 "[DzShaderManager] Failed to get DzShader {0}. Not found!",
                                 dzShaderName);

        return s_DzShaders[dzShaderName];
    }

    bool DzShaderManager::Compile()
    {
        bool hasError = false;

        for (const auto& [dzShaderName, dzShader] : s_DzShaders)
        {
            auto compileResult = s_Compiler.Compile(dzShader);
            if (!compileResult.Success)
            {
                SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] Failed to compile DzShader {0}, Message: {1}",
                                        dzShader.Name,
                                        compileResult.Message);
                hasError = true;
                continue;
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

    void DzShaderManager::SetInt(const std::string& dzShaderName, const std::string& propertyName, int value)
    {
        for (const auto& pass : s_DzShaders[dzShaderName].Passes)
        {
            auto& compiledShader = s_CompiledShaders[pass.Name];
            compiledShader->SetInt(propertyName, value);
        }
    }
} // namespace SnowLeopardEngine