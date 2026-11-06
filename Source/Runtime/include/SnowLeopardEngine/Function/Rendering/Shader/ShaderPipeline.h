#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"
#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderAsset.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace SnowLeopardEngine
{
    struct ShaderPipelineDesc
    {
        std::vector<VriFormat> ColorFormats {VriFormat_RGBA8_UNORM};
        VriFormat              DepthFormat = VriFormat_Unknown; // Unknown: no depth attachment (depth state ignored)
        VriPrimitiveTopology   Topology    = VriPrimitiveTopology_TriangleList;
        VriVertexInputDesc     VertexInput {}; // empty: vertices are generated in the shader
        std::string            Variant = "Default";
    };

    // The GPU side of one DzShader pass: pipeline layout from Slang reflection (descriptor-set index = register
    // space, push constants from [[vk::push_constant]]) and a graphics pipeline from the pass's render states.
    // `[property]` render states use the property's default value.
    class ShaderPipeline
    {
    public:
        ShaderPipeline(RHI::Device& device, const ShaderAsset& shader, const ShaderPass& pass, const ShaderPipelineDesc& desc);
        ~ShaderPipeline();

        ShaderPipeline(const ShaderPipeline&)            = delete;
        ShaderPipeline& operator=(const ShaderPipeline&) = delete;

        VriPipelineLayout* GetLayout() const { return m_Layout; }
        VriPipeline*       GetHandle() const { return m_Pipeline; }

        const std::vector<ShaderResourceBinding>& GetBindings() const { return m_Bindings; }
        // Index of `binding` within its descriptor set's ranges (for UpdateDescriptorRanges).
        uint32_t                RangeIndex(const ShaderResourceBinding& binding) const;
        std::optional<uint32_t> MaterialSet() const { return m_MaterialSet; } // set of `material`, if used
        uint32_t                PushConstantSize() const { return m_PushConstantSize; }
        const ShaderParameter*  MaterialLayout() const { return m_MaterialLayout; } // `material` element struct

        // Binds layout + pipeline (inside a render pass).
        void Bind(VriCommandBuffer* cmd) const;

    private:
        RHI::Device&                       m_Device;
        VriPipelineLayout*                 m_Layout   = nullptr;
        VriPipeline*                       m_Pipeline = nullptr;
        std::vector<ShaderResourceBinding> m_Bindings;
        std::map<std::pair<uint32_t, uint32_t>, uint32_t> m_RangeIndices; // (set, binding) -> range index
        std::optional<uint32_t>            m_MaterialSet;
        uint32_t                           m_PushConstantSize = 0;
        const ShaderParameter*             m_MaterialLayout   = nullptr;
        ShaderParameter                    m_Parameters; // reflection copy, owns m_MaterialLayout
    };

    // Values of a shader's `material` parameter block: a constant buffer laid out from reflection, plus the
    // textures and samplers of texture properties. Starts from the Properties defaults (builtin textures are
    // 1 x 1 white / black / flat normal).
    class ShaderMaterial
    {
    public:
        ShaderMaterial(RHI::Device& device, const ShaderAsset& shader, const ShaderPipeline& pipeline);
        ~ShaderMaterial();

        ShaderMaterial(const ShaderMaterial&)            = delete;
        ShaderMaterial& operator=(const ShaderMaterial&) = delete;

        // Throws if the property does not exist or the value does not fit it.
        void Set(std::string_view property, const ShaderPropertyValue& value);
        // The texture must stay alive and in the ShaderResource layout while the material is drawn.
        void SetTexture(std::string_view property, RHI::Texture& texture);

        // Writes pending values and binds the material's descriptor set (inside a render pass).
        void Bind(VriCommandBuffer* cmd);

    private:
        void Write(const ShaderProperty& property, const ShaderPropertyValue& value);
        void WriteTexture(const ShaderProperty& property, VriDescriptor* view);
        RHI::Texture& Builtin(const std::string& name);

    private:
        RHI::Device&           m_Device;
        const ShaderAsset&     m_Shader;
        const ShaderPipeline&  m_Pipeline;
        std::vector<std::byte> m_Uniforms;
        bool                   m_Dirty = true;
        Scope<RHI::Buffer>     m_UniformBuffer;
        VriDescriptor*         m_UniformView = nullptr;
        VriDescriptor*         m_Sampler     = nullptr;
        VriDescriptorPool*     m_Pool        = nullptr;
        VriDescriptorSet*      m_Set         = nullptr;
        std::map<std::string, Scope<RHI::Texture>, std::less<>> m_Builtins;
    };
} // namespace SnowLeopardEngine
