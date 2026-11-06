// DzShader v3 regression test.
//
// Parser: properties (all value kinds, attributes), variants, inherited render states, passes; errors carry
// file:line:column. Compiler: every pass x variant becomes SPIR-V with reflected bindings for `material`.
// GPU: a fullscreen-triangle pass drawn through the render graph and read back - material values, a texture
// property's builtin default, a variant define, a property-driven Cull state and push constants all change the
// pixels as expected. Exit code = number of failures.

#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderGraph/RenderGraph.h"
#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderAsset.h"
#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderPipeline.h"

#include <array>
#include <functional>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

using namespace SnowLeopardEngine;

namespace
{
    int g_Failures = 0;

    void Check(bool condition, const std::string& name, const std::string& detail = "")
    {
        std::printf("[%s] %-58s %s\n", condition ? "PASS" : "FAIL", name.c_str(), detail.c_str());
        if (!condition)
        {
            ++g_Failures;
        }
    }

    std::string ErrorOf(const std::function<void()>& f)
    {
        try
        {
            f();
        }
        catch (const std::exception& error)
        {
            return error.what();
        }
        return {};
    }

    const char* kShader = R"(Shader "Tests/Fill"
{
    Properties
    {
        tint ("Tint", Color) = (1, 0.5, 0.25, 1)
        gain ("Gain", Range(0, 2)) = 1
        [Enum(Off, 0, Front, 1, Back, 2)] cull ("Cull", Integer) = 2
        enabled ("Enabled", Boolean) = true
        albedo ("Albedo", 2D) = "white" {}
        [HideInInspector] unused ("Unused", Float) = 3
    }
    Variant "Default" { }
    Variant "Inverted" { Defines { INVERT = 1 } }
    SubShader
    {
        Tags { "RenderType" = "Opaque" }
        Cull [cull]
        ZWrite Off
        SLANGINCLUDE
        struct VertexOutput { float4 position : SV_Position; float2 uv : TEXCOORD0; };
        VertexOutput fullscreen(uint id)
        {
            // CCW in a Y-up clip space: (-1, -1), (3, -1), (-1, 3)
            float2 position = float2(id == 1 ? 3.0 : -1.0, id == 2 ? 3.0 : -1.0);
            VertexOutput output;
            output.position = float4(position, 0, 1);
            output.uv       = position * 0.5 + 0.5;
            return output;
        }
        ENDSLANG
        Pass
        {
            Name "Fill"
            Tags { "LightMode" = "Forward" }
            Vertex vertexMain
            Fragment fragmentMain
            SLANGPROGRAM
            VertexOutput vertexMain(uint id : SV_VertexID) { return fullscreen(id); }
            float4 fragmentMain(VertexOutput input) : SV_Target0
            {
                float3 color = material.tint.rgb * material.gain * material.albedo.Sample(material.albedoSampler, input.uv).rgb;
                if (!material.enabled) { color = float3(0, 0, 0); }
            #ifdef INVERT
                color = 1.0 - color;
            #endif
                return float4(color, 1);
            }
            ENDSLANG
        }
        Pass
        {
            Name "Offset"
            Cull Off
            Vertex vertexMain
            Fragment fragmentMain
            SLANGPROGRAM
            struct Offset { float4 value; };
            [[vk::push_constant]] ConstantBuffer<Offset> offset;
            VertexOutput vertexMain(uint id : SV_VertexID) { return fullscreen(id); }
            float4 fragmentMain(VertexOutput input) : SV_Target0 { return float4(offset.value.rgb, 1); }
            ENDSLANG
        }
    }
}
)";

    constexpr RHI::Extent kExtent {8, 8};

    struct Pixel
    {
        uint8_t R, G, B, A;
        std::string Text() const
        {
            return "(" + std::to_string(R) + ", " + std::to_string(G) + ", " + std::to_string(B) + ", " + std::to_string(A) + ")";
        }
        bool Near(int r, int g, int b) const { return std::abs(R - r) <= 2 && std::abs(G - g) <= 2 && std::abs(B - b) <= 2 && A == 255; }
    };

    // Clears an 8 x 8 target to dark grey, runs `draw` in a render pass on it, returns the centre pixel.
    Pixel DrawAndRead(RHI::Device& device, const std::function<void(VriCommandBuffer*)>& draw)
    {
        RenderGraph graph(device);
        auto        target = graph.CreateTexture("Target", RHI::ColorTextureDesc(kExtent, VriFormat_RGBA8_UNORM));
        graph.AddPass("Draw", {{target, GraphUsage::ColorWrite, true}}, [&](VriCommandBuffer* cmd, RenderGraph& g) {
            auto&       texture = g.GetTexture(target);
            const float clear[4] {0.1f, 0.1f, 0.1f, 1.0f};
            RHI::BeginColorPass(device, cmd, texture.View(), texture.GetExtent(), clear);
            draw(cmd);
            device.Core.CmdEndRendering(cmd);
        });
        graph.ExportResource(target);
        graph.Compile();

        RHI::Frame frame(device);
        auto*      cmd = frame.Begin();
        graph.Execute(cmd);
        auto&                texture = graph.GetTexture(target);
        RHI::TextureReadback readback(device, texture);
        readback.Record(cmd, texture);
        frame.SubmitAndWait();
        const auto pixels = readback.Pixels();
        const auto center = (size_t(kExtent.Height / 2) * kExtent.Width + kExtent.Width / 2) * 4;
        return {pixels[center], pixels[center + 1], pixels[center + 2], pixels[center + 3]};
    }

    void Triangle(RHI::Device& device, VriCommandBuffer* cmd)
    {
        const VriDrawDesc draw {3, 1, 0, 0};
        device.Core.CmdDraw(cmd, &draw);
    }
} // namespace

int main() TRY
{
    EngineInitInfo initInfo {};
    initInfo.Window.Title = "Test - DzShader";

    Engine engine;
    if (!engine.Init(initInfo))
    {
        std::fprintf(stderr, "Failed to init engine!\n");
        return 1;
    }
    auto&      device = g_EngineContext->RenderSys->GetDevice();
    const auto path   = std::filesystem::path("Tests/Fill.dzshader");

    // ---- parser
    const auto parsed = ShaderAsset::Parse(path, kShader);
    Check(parsed.Name == "Tests/Fill", "shader name", parsed.Name);
    Check(parsed.Properties.size() == 6, "six properties", std::to_string(parsed.Properties.size()));
    const auto& tint = parsed.GetProperty("tint");
    Check(tint.Type == ShaderPropertyType::Color &&
              std::get<std::array<float, 4>>(tint.DefaultValue) == std::array<float, 4> {1, 0.5f, 0.25f, 1},
          "Color default");
    const auto& gain = parsed.GetProperty("gain");
    Check(gain.Type == ShaderPropertyType::Range && gain.Minimum == 0 && gain.Maximum == 2, "Range bounds");
    const auto& cull = parsed.GetProperty("cull");
    Check(cull.Enumeration.size() == 3 && std::get<int32_t>(cull.DefaultValue) == 2, "Enum attribute and Integer default");
    Check(std::get<ShaderTextureValue>(parsed.GetProperty("albedo").DefaultValue).Builtin == "white", "texture builtin default");
    Check(parsed.GetProperty("unused").Hidden, "HideInInspector attribute");
    Check(parsed.Variants.size() == 2 && parsed.GetVariant("Inverted").Defines.at(0).Name == "INVERT", "variants and defines");
    const auto& subshader = parsed.Subshaders.at(0);
    Check(subshader.Tags.at("RenderType") == "Opaque" && subshader.Common.size() == 1, "SubShader tags and SLANGINCLUDE");
    const auto* fillPass   = subshader.FindPass("Fill");
    const auto* offsetPass = subshader.FindPass("Offset");
    Check(fillPass != nullptr && offsetPass != nullptr, "both passes found");
    Check(fillPass->LightMode == "Forward" && fillPass->Entries.size() == 2, "pass LightMode and entries");
    Check(fillPass->State.Commands.at("Cull").at(0).Property && fillPass->State.Commands.at("ZWrite").at(0).Value == "Off",
          "pass inherits SubShader states");
    Check(offsetPass->State.Commands.at("Cull").at(0).Value == "Off", "pass overrides an inherited state");

    // ---- parse errors carry the location
    auto broken = std::string(kShader);
    broken.replace(broken.find("Properties"), 10, "Propertie");
    auto error = ErrorOf([&] { ShaderAsset::Parse(path, broken); });
    Check(error.find("Fill.dzshader:3:") != std::string::npos, "syntax error names file and line", error.substr(0, 90));
    error = ErrorOf([&] {
        auto text = std::string(kShader);
        text.replace(text.find("Range(0, 2)) = 1"), 17, "Range(0, 2)) = 5");
        ShaderAsset::Parse(path, text);
    });
    Check(error.find("outside its Range") != std::string::npos, "default outside its Range is rejected");
    error = ErrorOf([&] {
        auto text = std::string(kShader);
        text.replace(text.find("ZWrite Off"), 10, "ZWrite Of ");
        ShaderAsset::Parse(path, text);
    });
    Check(error.find("invalid value Of for ZWrite") != std::string::npos, "bad render state value is rejected", error.substr(0, 90));
    error = ErrorOf([&] {
        auto text = std::string(kShader);
        text.replace(text.find("Float) = 3"), 10, "Floaty) = 3");
        ShaderAsset::Parse(path, text);
    });
    Check(error.find("unknown property type Floaty") != std::string::npos, "unknown property type is rejected");

    // ---- compile
    const auto shader = ShaderAsset::LoadSource(path, kShader);
    const auto& fill  = *shader.Subshaders[0].FindPass("Fill");
    Check(fill.Programs.size() == 2, "one program per variant");
    const auto& program = fill.GetProgram();
    Check(program.Shaders.size() == 2 && program.Shaders[0].Words.at(0) == 0x07230203, "SPIR-V for both stages");
    const auto bindings = program.ResourceBindings();
    auto       has      = [&](const std::string& name, VriDescriptorType type) {
        return std::any_of(bindings.begin(), bindings.end(), [&](const auto& b) { return b.Name == name && b.Type == type; });
    };
    Check(has("material", VriDescriptorType_ConstantBuffer) && has("material.albedo", VriDescriptorType_Texture) &&
              has("material.albedoSampler", VriDescriptorType_Sampler),
          "reflected material constant buffer, texture and sampler");
    const auto& offsetProgram = shader.Subshaders[0].FindPass("Offset")->GetProgram();
    const auto  offsetBindings = offsetProgram.ResourceBindings();
    Check(std::any_of(offsetBindings.begin(), offsetBindings.end(), [](const auto& b) { return b.PushConstant && b.UniformSize == 16; }),
          "push constant block reflected");
    error = ErrorOf([&] {
        auto text = std::string(kShader);
        text.replace(text.find("material.gain *"), 15, "material.gainz*");
        ShaderAsset::LoadSource(path, text);
    });
    Check(error.find("Fill.dzshader") != std::string::npos && error.find("gainz") != std::string::npos,
          "Slang errors point into the .dzshader", error.substr(0, 90));

    // ---- GPU
    {
        ShaderPipeline pipeline(device, shader, fill, {});
        ShaderMaterial material(device, shader, pipeline);
        Check(pipeline.MaterialSet().has_value() && pipeline.MaterialLayout() != nullptr, "pipeline finds the material block");

        auto draw = [&](ShaderMaterial& m, ShaderPipeline& p) {
            return DrawAndRead(device, [&](VriCommandBuffer* cmd) {
                p.Bind(cmd);
                m.Bind(cmd);
                Triangle(device, cmd);
            });
        };
        auto pixel = draw(material, pipeline);
        Check(pixel.Near(255, 128, 64), "defaults: tint x gain x white", pixel.Text());

        material.Set("gain", 0.5f);
        material.Set("tint", std::array<float, 4> {0, 1, 1, 1});
        pixel = draw(material, pipeline);
        Check(pixel.Near(0, 128, 128), "changed material values reach the shader", pixel.Text());

        material.Set("albedo", ShaderTextureValue {"black"});
        pixel = draw(material, pipeline);
        Check(pixel.Near(0, 0, 0), "texture property switched to the black builtin", pixel.Text());

        material.Set("albedo", ShaderTextureValue {"white"});
        material.Set("enabled", false);
        pixel = draw(material, pipeline);
        Check(pixel.Near(0, 0, 0), "Boolean property reaches the shader", pixel.Text());

        Check(!ErrorOf([&] { material.Set("gain", 3.0f); }).empty(), "value outside its Range is rejected");
        Check(!ErrorOf([&] { material.Set("tint", 1.0f); }).empty(), "value of the wrong type is rejected");
    }
    {
        ShaderPipelineDesc desc;
        desc.Variant = "Inverted";
        ShaderPipeline pipeline(device, shader, fill, desc);
        ShaderMaterial material(device, shader, pipeline);
        const auto     pixel = DrawAndRead(device, [&](VriCommandBuffer* cmd) {
            pipeline.Bind(cmd);
            material.Bind(cmd);
            Triangle(device, cmd);
        });
        Check(pixel.Near(0, 127, 191), "variant define inverts the colour", pixel.Text());
    }
    {
        // Cull [cull] with the property's default 1 = Front: the CCW triangle is culled, the clear colour stays.
        auto text = std::string(kShader);
        text.replace(text.find("(\"Cull\", Integer) = 2"), 21, "(\"Cull\", Integer) = 1");
        const auto     culled = ShaderAsset::LoadSource(path, text);
        ShaderPipeline pipeline(device, culled, *culled.Subshaders[0].FindPass("Fill"), {});
        ShaderMaterial material(device, culled, pipeline);
        const auto     pixel = DrawAndRead(device, [&](VriCommandBuffer* cmd) {
            pipeline.Bind(cmd);
            material.Bind(cmd);
            Triangle(device, cmd);
        });
        Check(pixel.Near(26, 26, 26), "Cull [cull] = Front culls the front-facing triangle", pixel.Text());
    }
    {
        ShaderPipeline pipeline(device, shader, *shader.Subshaders[0].FindPass("Offset"), {});
        // The generated `material` block is declared (and reflected) in every pass, even one that never reads it.
        Check(pipeline.PushConstantSize() == 16, "16-byte push constant range", std::to_string(pipeline.PushConstantSize()));
        const auto pixel = DrawAndRead(device, [&](VriCommandBuffer* cmd) {
            pipeline.Bind(cmd);
            const float value[4] {0.2f, 0.4f, 0.6f, 1.0f};
            device.Core.CmdSetConstants(cmd, 0, value, sizeof(value));
            Triangle(device, cmd);
        });
        Check(pixel.Near(51, 102, 153), "push constants reach the shader", pixel.Text());
    }

    std::printf("\n%d failure(s)\n", g_Failures);
    engine.Shutdown();
    return g_Failures;
}
CATCH
