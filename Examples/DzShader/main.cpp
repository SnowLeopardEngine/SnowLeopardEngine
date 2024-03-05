#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

#include <fstream>

using namespace SnowLeopardEngine;

int main()
{
    std::stringstream ss;

    {
        DzShaderLanguageType languageType = DzShaderLanguageType::GLSL;

        DzStage vertexStage      = {};
        vertexStage.Name         = "vertex";
        vertexStage.LanguageType = magic_enum::enum_name(languageType);
        vertexStage.EntryPoint   = "main"; // optional
        vertexStage.ShaderSource = R"(
#version 330

layout(location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

        DzStage fragmentStage      = {};
        fragmentStage.Name         = "fragment";
        fragmentStage.LanguageType = magic_enum::enum_name(languageType);
        fragmentStage.EntryPoint   = "main"; // optional
        fragmentStage.ShaderSource = R"(
#version 330

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(1, 1, 1, 1);
}
)";

        DzPass geometryPass                      = {};
        geometryPass.Name                        = "GeometryPass";
        geometryPass.PipelineStates.CullFaceMode = "Back";
        geometryPass.PipelineStates.ZTest        = "On";
        geometryPass.PipelineStates.ZTestMode    = "LessEqual";
        geometryPass.Tags.RenderQueue            = "Geometry";
        geometryPass.Stages.emplace_back(vertexStage);
        geometryPass.Stages.emplace_back(fragmentStage);

        DzShader shader = {};
        shader.Name     = "Opaque/Legacy";
        shader.Properties.emplace_back(
            DzShaderProperty("BaseColor", magic_enum::enum_name(DzShaderPropertyType::Color), "(1, 1, 1, 1)"));
        shader.Properties.emplace_back(
            DzShaderProperty("Diffuse", magic_enum::enum_name(DzShaderPropertyType::Texture2D), ""));
        shader.PipelineStates.ZWrite = "On";
        shader.Passes.emplace_back(geometryPass);
        cereal::JSONOutputArchive oarchive(ss);

        oarchive(cereal::make_nvp("DzShader", shader));
    }

    std::cout << ss.str() << std::endl;

    {
        cereal::JSONInputArchive iarchive(ss);
        DzShader                 shader;
        iarchive(shader);

        auto compileResult = DzShaderCompiler::Compile(shader);
        if (!compileResult.Success)
        {
            std::cerr << compileResult.Message << std::endl;
            return 1;
        }

        for (const auto& passResult : compileResult.PassResults)
        {
            for (const auto& [stageName, glslSource] : passResult.ShaderStageSources)
            {
                std::cout << "Stage: " << stageName << ", Source:\n" << glslSource << std::endl;
            }
        }
    }

    std::ifstream shaderFile("shaders/test.dzshader");
    cereal::JSONInputArchive archive(shaderFile);

    DzShader shaderFromFile;
    archive(shaderFromFile);

    return 0;
}