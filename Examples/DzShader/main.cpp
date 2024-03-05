#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

using namespace SnowLeopardEngine;

int main()
{
    std::stringstream ss;

    {
        DzShaderLanguageType languageType = DzShaderLanguageType::GLSL;

        DzStage vertexStage      = {};
        vertexStage.Name         = "vertex";
        vertexStage.LanguageType = languageType;
        vertexStage.EntryPoint   = "main"; // optional
        vertexStage.ShaderSource = R"(
#version 450

layout(location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

        DzStage fragmentStage      = {};
        fragmentStage.Name         = "Fragment";
        fragmentStage.LanguageType = languageType;
        fragmentStage.EntryPoint   = "main"; // optional
        fragmentStage.ShaderSource = R"(
#version 450

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(1, 1, 1, 1);
}
)";

        DzPass geometryPass                  = {};
        geometryPass.Name                    = "GeometryPass";
        geometryPass.PipelineStates["Cull"]  = "Back";
        geometryPass.PipelineStates["ZTest"] = "On";
        geometryPass.Tags["RenderQueue"]     = "Geometry";
        geometryPass.Stages.emplace_back(vertexStage);
        geometryPass.Stages.emplace_back(fragmentStage);

        DzShader shader = {};
        shader.Name     = "Opaque/Legacy";
        shader.Properties.emplace_back(DzShaderProperty("BaseColor", DzShaderPropertyType::Color, "(1, 1, 1, 1)"));
        shader.Properties.emplace_back(DzShaderProperty("Diffuse", DzShaderPropertyType::Texture2D, ""));
        shader.PipelineStates["ZWrite"] = "On";
        shader.Passes.emplace_back(geometryPass);
        cereal::JSONOutputArchive oarchive(ss);

        oarchive(cereal::make_nvp("DzShader", shader));
    }

    {
        cereal::JSONInputArchive iarchive(ss);
        DzShader                 shader;
        iarchive(shader);

        // Add a break point here.
    }

    std::cout << ss.str() << std::endl;

    // TODO: DzShader Compiler get result

    return 0;
}