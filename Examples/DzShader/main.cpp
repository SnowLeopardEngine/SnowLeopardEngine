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
#version 450

#include "common.vert"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

layout(location = 0) out vec2 v2fTexCoords;
layout(location = 1) out vec3 v2fCameraPos;

void main()
{
    v2fTexCoords = aTexCoords; // Varying
    v2fCameraPos = uScene.cameraPos; // Varying
    gl_Position = uScene.projection * uScene.view * uScene.model * vec4(aPos, 1.0);
}
)";

        DzStage fragmentStage      = {};
        fragmentStage.Name         = "fragment";
        fragmentStage.LanguageType = magic_enum::enum_name(languageType);
        fragmentStage.EntryPoint   = "main"; // optional
        fragmentStage.ShaderSource = R"(
#version 450

layout(location = 0) in vec2 v2fTexCoords;
layout(location = 1) in vec3 v2fCameraPos;

layout(location = 0) out vec4 fragColor;

uniform vec4 BaseColor;
uniform sampler2D Diffuse;

void main()
{
    vec4 diffuseColor = texture(Diffuse, v2fTexCoords);
    fragColor = mix(diffuseColor, BaseColor, 0.5);
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

        std::cout << "Compile Result:" << std::endl;

        for (const auto& passResult : compileResult.PassResults)
        {
            for (const auto& [stageName, glslSource] : passResult.ShaderStageSources)
            {
                std::cout << "Stage: " << stageName << ", Source:\n\n" << glslSource << std::endl;
            }
            std::cout << std::endl;
        }
    }

    std::ifstream            shaderFile("shaders/test.dzshader");
    cereal::JSONInputArchive archive(shaderFile);

    DzShader shaderFromFile;
    archive(shaderFromFile);

    return 0;
}