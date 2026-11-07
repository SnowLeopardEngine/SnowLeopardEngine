#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderPipeline.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <set>
#include <stdexcept>

namespace SnowLeopardEngine
{
    namespace
    {
        const std::vector<std::string>& AllowedValues(std::string_view command)
        {
            static const std::vector<std::string> cull {"Off", "Front", "Back"};
            static const std::vector<std::string> frontFace {"CW", "CCW"};
            static const std::vector<std::string> onOff {"On", "Off"};
            static const std::vector<std::string> compare {"Never", "Less", "Equal", "LEqual", "Greater", "NotEqual", "GEqual", "Always"};
            static const std::vector<std::string> blend {"Off",
                                                         "Zero",
                                                         "One",
                                                         "SrcColor",
                                                         "OneMinusSrcColor",
                                                         "DstColor",
                                                         "OneMinusDstColor",
                                                         "SrcAlpha",
                                                         "OneMinusSrcAlpha",
                                                         "DstAlpha",
                                                         "OneMinusDstAlpha",
                                                         "SrcAlphaSaturate",
                                                         "ConstantColor",
                                                         "OneMinusConstantColor",
                                                         "ConstantAlpha",
                                                         "OneMinusConstantAlpha"};
            static const std::vector<std::string> blendOp {"Add", "Subtract", "ReverseSubtract", "Min", "Max"};
            static const std::vector<std::string> none;
            if (command == "Cull")
            {
                return cull;
            }
            if (command == "FrontFace")
            {
                return frontFace;
            }
            if (command == "ZWrite")
            {
                return onOff;
            }
            if (command == "ZTest")
            {
                return compare;
            }
            if (command == "Blend")
            {
                return blend;
            }
            if (command == "BlendOp")
            {
                return blendOp;
            }
            return none;
        }

        // A state value as text; `[property]` values pick from the allowed values by the property's default
        // (Enum(Off, 0, Front, 1, Back, 2) style), or give its number for numeric states.
        std::string Resolve(const ShaderAsset& shader, std::string_view command, const ShaderStateValue& value)
        {
            if (!value.Property)
            {
                return value.Value;
            }
            const auto& property = shader.GetProperty(value.Value);
            double      number   = 0;
            if (const auto* integer = std::get_if<int32_t>(&property.DefaultValue))
            {
                number = *integer;
            }
            else if (const auto* scalar = std::get_if<float>(&property.DefaultValue))
            {
                number = *scalar;
            }
            const auto& allowed = AllowedValues(command);
            if (allowed.empty())
            {
                return std::to_string(number);
            }
            const auto index = int64_t(std::llround(number));
            if (index < 0 || index >= int64_t(allowed.size()))
            {
                throw std::invalid_argument("Property " + property.Name + " selects no valid " + std::string(command));
            }
            return allowed[size_t(index)];
        }

        VriCompareOp ToCompare(const std::string& value)
        {
            if (value == "Never")
                return VriCompareOp_Never;
            if (value == "Less")
                return VriCompareOp_Less;
            if (value == "Equal")
                return VriCompareOp_Equal;
            if (value == "LEqual")
                return VriCompareOp_LessOrEqual;
            if (value == "Greater")
                return VriCompareOp_Greater;
            if (value == "NotEqual")
                return VriCompareOp_NotEqual;
            if (value == "GEqual")
                return VriCompareOp_GreaterOrEqual;
            return VriCompareOp_Always;
        }

        VriBlendFactor ToBlendFactor(const std::string& value)
        {
            static const std::map<std::string, VriBlendFactor, std::less<>> factors {
                {"Zero", VriBlendFactor_Zero},
                {"One", VriBlendFactor_One},
                {"SrcColor", VriBlendFactor_SrcColor},
                {"OneMinusSrcColor", VriBlendFactor_OneMinusSrcColor},
                {"DstColor", VriBlendFactor_DstColor},
                {"OneMinusDstColor", VriBlendFactor_OneMinusDstColor},
                {"SrcAlpha", VriBlendFactor_SrcAlpha},
                {"OneMinusSrcAlpha", VriBlendFactor_OneMinusSrcAlpha},
                {"DstAlpha", VriBlendFactor_DstAlpha},
                {"OneMinusDstAlpha", VriBlendFactor_OneMinusDstAlpha},
                {"SrcAlphaSaturate", VriBlendFactor_SrcAlphaSaturate},
                {"ConstantColor", VriBlendFactor_ConstantColor},
                {"OneMinusConstantColor", VriBlendFactor_OneMinusConstantColor},
                {"ConstantAlpha", VriBlendFactor_ConstantAlpha},
                {"OneMinusConstantAlpha", VriBlendFactor_OneMinusConstantAlpha}};
            const auto found = factors.find(value);
            if (found == factors.end())
            {
                throw std::invalid_argument("Unknown blend factor " + value);
            }
            return found->second;
        }

        VriBlendOp ToBlendOp(const std::string& value)
        {
            if (value == "Subtract")
                return VriBlendOp_Subtract;
            if (value == "ReverseSubtract")
                return VriBlendOp_ReverseSubtract;
            if (value == "Min")
                return VriBlendOp_Min;
            if (value == "Max")
                return VriBlendOp_Max;
            return VriBlendOp_Add;
        }

        const ShaderParameter* FindMaterial(const ShaderParameter& globals)
        {
            // $globals is a struct (or a constant buffer wrapping one) holding `material`.
            const ShaderParameter* node = &globals;
            if (node->Kind == ShaderParameterKind::ConstantBuffer || node->Kind == ShaderParameterKind::ParameterBlock)
            {
                node = node->Field("$element");
            }
            if (node == nullptr)
            {
                return nullptr;
            }
            const auto* material = node->Field("material");
            if (material == nullptr || material->Kind != ShaderParameterKind::ParameterBlock)
            {
                return nullptr;
            }
            return material->Field("$element");
        }
    } // namespace

    // ---------------------------------------------------------------- ShaderPipeline

    ShaderPipeline::ShaderPipeline(RHI::Device& device, const ShaderAsset& shader, const ShaderPass& pass, const ShaderPipelineDesc& desc) :
        m_Device(device)
    {
        const auto& program = pass.GetProgram(desc.Variant);
        m_Parameters        = program.Parameters;
        m_MaterialLayout    = FindMaterial(m_Parameters);
        m_Bindings          = program.ResourceBindings();

        VriShaderStageFlags stages = 0;
        for (const auto& entry : pass.Entries)
        {
            stages |= entry.Stage;
        }
        m_Compute = (stages & VriShaderStage_Compute) != 0;
        if (m_Compute && stages != VriShaderStage_Compute)
        {
            throw std::invalid_argument("Pass " + pass.Name + " mixes compute and graphics stages");
        }

        // Descriptor sets: one VRI set per register space, the array index equals the space (empty sets fill gaps).
        uint32_t                                   setCount = 0;
        std::map<uint32_t, std::vector<VriDescriptorRangeDesc>> ranges;
        std::vector<VriPushConstantDesc>           pushConstants;
        for (const auto& binding : m_Bindings)
        {
            if (binding.PushConstant)
            {
                if (!pushConstants.empty())
                {
                    throw std::invalid_argument("Pass " + pass.Name + " declares more than one push-constant block");
                }
                m_PushConstantSize = uint32_t((binding.UniformSize + 3) / 4 * 4);
                pushConstants.push_back({0, m_PushConstantSize, stages});
                continue;
            }
            VriDescriptorRangeDesc range {};
            range.baseRegister   = binding.Binding;
            range.descriptorNum  = binding.Count;
            range.descriptorType = binding.Type;
            range.shaderStages   = stages;
            range.viewType       = binding.ViewType;
            if (binding.Count > 1)
            {
                range.flags = VriDescriptorRange_PartiallyBound;
            }
            m_RangeIndices[{binding.Set, binding.Binding}] = uint32_t(ranges[binding.Set].size());
            ranges[binding.Set].push_back(range);
            setCount = std::max(setCount, binding.Set + 1);
            if (binding.Name == "material" || binding.Name.starts_with("material."))
            {
                if (m_MaterialSet && *m_MaterialSet != binding.Set)
                {
                    throw std::logic_error("material block spans several descriptor sets");
                }
                m_MaterialSet = binding.Set;
            }
        }
        std::vector<VriDescriptorSetDesc> sets(setCount);
        for (uint32_t set = 0; set < setCount; ++set)
        {
            sets[set].registerSpace = set;
            if (auto found = ranges.find(set); found != ranges.end())
            {
                sets[set].ranges   = found->second.data();
                sets[set].rangeNum = uint32_t(found->second.size());
            }
        }
        VriPipelineLayoutDesc layout {};
        layout.descriptorSets   = sets.data();
        layout.descriptorSetNum = setCount;
        layout.pushConstants    = pushConstants.data();
        layout.pushConstantNum  = uint32_t(pushConstants.size());
        layout.shaderStages     = stages;
        RHI::Check(device.Core.CreatePipelineLayout(device.Handle, &layout, &m_Layout), "Create pipeline layout");

        if (m_Compute)
        {
            const auto             shaders = program.Descriptors(pass.Entries);
            VriComputePipelineDesc compute {};
            compute.pipelineLayout = m_Layout;
            compute.shader         = shaders.at(0);
            compute.pipelineCache  = device.PipelineCache;
            if (const auto result = device.Core.CreateComputePipeline(device.Handle, &compute, &m_Pipeline);
                result != VriResult_Success)
            {
                device.Core.DestroyPipelineLayout(m_Layout);
                RHI::Check(result, "Create compute pipeline");
            }
            return;
        }

        try
        {
            const auto& state  = pass.State.Commands;
            auto        values = [&](std::string_view command) -> std::vector<std::string> {
                std::vector<std::string> result;
                if (const auto found = state.find(command); found != state.end())
                {
                    for (const auto& value : found->second)
                    {
                        result.push_back(Resolve(shader, command, value));
                    }
                }
                return result;
            };

            VriGraphicsPipelineDesc pipeline {};
            pipeline.pipelineCache          = device.PipelineCache;
            pipeline.pipelineLayout         = m_Layout;
            const auto shaders              = program.Descriptors(pass.Entries);
            pipeline.shaders                = shaders.data();
            pipeline.shaderNum              = uint32_t(shaders.size());
            pipeline.vertexInput            = desc.VertexInput;
            pipeline.inputAssembly.topology = desc.Topology;
            pipeline.rasterization.lineWidth = 1;
            pipeline.multisample.sampleNum   = 1;

            // Cull (default Back), FrontFace (default CCW)
            pipeline.rasterization.cullMode  = VriCullMode_Back;
            pipeline.rasterization.frontFace = VriFrontFace_CounterClockwise;
            if (const auto cull = values("Cull"); !cull.empty())
            {
                pipeline.rasterization.cullMode = cull[0] == "Off" ? VriCullMode_None : cull[0] == "Front" ? VriCullMode_Front : VriCullMode_Back;
            }
            if (const auto front = values("FrontFace"); !front.empty())
            {
                pipeline.rasterization.frontFace = front[0] == "CW" ? VriFrontFace_Clockwise : VriFrontFace_CounterClockwise;
            }
            if (const auto bias = values("DepthBias"); !bias.empty())
            {
                pipeline.rasterization.depthBias.constant = std::stof(bias[0]);
                pipeline.rasterization.depthBias.slope    = bias.size() > 1 ? std::stof(bias[1]) : 0.0f;
                pipeline.rasterization.depthBias.clamp    = bias.size() > 2 ? std::stof(bias[2]) : 0.0f;
            }

            // Depth: test LEqual and write On by default, when there is a depth attachment.
            if (desc.DepthFormat != VriFormat_Unknown)
            {
                pipeline.depthStencil.depthTest      = VRI_TRUE;
                pipeline.depthStencil.depthWrite     = VRI_TRUE;
                pipeline.depthStencil.depthCompareOp = VriCompareOp_LessOrEqual;
                if (const auto write = values("ZWrite"); !write.empty())
                {
                    pipeline.depthStencil.depthWrite = write[0] == "On" ? VRI_TRUE : VRI_FALSE;
                }
                if (const auto test = values("ZTest"); !test.empty())
                {
                    pipeline.depthStencil.depthCompareOp = ToCompare(test[0]);
                }
            }
            if (state.contains("Stencil"))
            {
                throw std::invalid_argument(pass.Location.Describe() + ": Stencil states are not supported yet");
            }

            // Blend and colour mask apply to every colour attachment.
            VriColorAttachmentDesc color {};
            color.colorWriteMask = VriColorWrite_RGBA;
            if (const auto blend = values("Blend"); !blend.empty() && blend[0] != "Off")
            {
                color.blend.enable   = VRI_TRUE;
                color.blend.srcColor = ToBlendFactor(blend[0]);
                color.blend.dstColor = ToBlendFactor(blend.at(1));
                color.blend.srcAlpha = blend.size() == 4 ? ToBlendFactor(blend[2]) : color.blend.srcColor;
                color.blend.dstAlpha = blend.size() == 4 ? ToBlendFactor(blend[3]) : color.blend.dstColor;
                color.blend.colorOp  = VriBlendOp_Add;
                color.blend.alphaOp  = VriBlendOp_Add;
                if (const auto op = values("BlendOp"); !op.empty())
                {
                    color.blend.colorOp = ToBlendOp(op[0]);
                    color.blend.alphaOp = ToBlendOp(op.size() > 1 ? op[1] : op[0]);
                }
            }
            if (const auto mask = values("ColorMask"); !mask.empty())
            {
                color.colorWriteMask = VriColorWrite_None;
                for (const char channel : mask[0])
                {
                    color.colorWriteMask |= channel == 'R' ? VriColorWrite_R :
                                            channel == 'G' ? VriColorWrite_G :
                                            channel == 'B' ? VriColorWrite_B :
                                            channel == 'A' ? VriColorWrite_A :
                                                             VriColorWrite_None;
                }
            }
            std::vector<VriColorAttachmentDesc> colors;
            for (const auto format : desc.ColorFormats)
            {
                auto attachment   = color;
                attachment.format = format;
                colors.push_back(attachment);
            }
            if (const auto* fragment = program.Find(VriShaderStage_Fragment);
                fragment != nullptr && fragment->ColorOutputs > colors.size())
            {
                throw std::invalid_argument("Pass " + pass.Name + " writes " + std::to_string(fragment->ColorOutputs) +
                                            " colour outputs but the pipeline has " + std::to_string(colors.size()) + " targets");
            }
            pipeline.outputMerger.colors             = colors.data();
            pipeline.outputMerger.colorNum           = uint32_t(colors.size());
            pipeline.outputMerger.depthStencilFormat = desc.DepthFormat;
            RHI::Check(device.Core.CreateGraphicsPipeline(device.Handle, &pipeline, &m_Pipeline), "Create graphics pipeline");
        }
        catch (...)
        {
            device.Core.DestroyPipelineLayout(m_Layout);
            throw;
        }
    }

    ShaderPipeline::~ShaderPipeline()
    {
        m_Device.WaitIdle();
        if (m_Pipeline != nullptr)
        {
            m_Device.Core.DestroyPipeline(m_Pipeline);
        }
        if (m_Layout != nullptr)
        {
            m_Device.Core.DestroyPipelineLayout(m_Layout);
        }
    }

    uint32_t ShaderPipeline::RangeIndex(const ShaderResourceBinding& binding) const
    {
        const auto found = m_RangeIndices.find({binding.Set, binding.Binding});
        if (found == m_RangeIndices.end())
        {
            throw std::invalid_argument("No descriptor range for " + binding.Name);
        }
        return found->second;
    }

    void ShaderPipeline::Bind(VriCommandBuffer* cmd) const
    {
        m_Device.Core.CmdSetPipelineLayout(cmd, m_Layout);
        m_Device.Core.CmdSetPipeline(cmd, m_Pipeline);
    }

    void ShaderPipeline::SetConstants(VriCommandBuffer* cmd, const void* data, uint32_t size) const
    {
        if (size > m_PushConstantSize)
        {
            throw std::invalid_argument("Push constants larger than the shader's block");
        }
        m_Device.Core.CmdSetConstants(cmd, 0, data, size);
    }

    // ---------------------------------------------------------------- ShaderBindings

    ShaderBindings::ShaderBindings(RHI::Device& device, const ShaderPipeline& pipeline, uint32_t set) :
        m_Device(device), m_Pipeline(pipeline), m_SetIndex(set)
    {
        VriDescriptorPoolDesc pool {};
        pool.descriptorSetMaxNum = 1;
        bool any                 = false;
        for (const auto& binding : pipeline.GetBindings())
        {
            if (binding.PushConstant || binding.Set != set)
            {
                continue;
            }
            any = true;
            switch (binding.Type)
            {
                case VriDescriptorType_Sampler:
                    pool.samplerMaxNum += binding.Count;
                    break;
                case VriDescriptorType_Texture:
                    pool.textureMaxNum += binding.Count;
                    break;
                case VriDescriptorType_StorageTexture:
                    pool.storageTextureMaxNum += binding.Count;
                    break;
                case VriDescriptorType_ConstantBuffer:
                    pool.constantBufferMaxNum += binding.Count;
                    break;
                case VriDescriptorType_StructuredBuffer:
                    pool.structuredBufferMaxNum += binding.Count;
                    break;
                case VriDescriptorType_StorageBuffer:
                    pool.storageBufferMaxNum += binding.Count;
                    break;
                default:
                    throw std::invalid_argument("Unsupported descriptor type for " + binding.Name);
            }
        }
        if (!any)
        {
            return;
        }
        RHI::Check(device.Core.CreateDescriptorPool(device.Handle, &pool, &m_Pool), "Create descriptor pool");
        RHI::Check(device.Core.AllocateDescriptorSets(m_Pool, pipeline.GetLayout(), set, &m_Set, 1), "Allocate descriptor set");
    }

    ShaderBindings::~ShaderBindings()
    {
        if (m_Pool != nullptr)
        {
            m_Device.Core.DestroyDescriptorPool(m_Pool);
        }
    }

    bool ShaderBindings::Has(std::string_view name) const
    {
        return std::any_of(m_Pipeline.GetBindings().begin(), m_Pipeline.GetBindings().end(), [&](const auto& binding) {
            return !binding.PushConstant && binding.Set == m_SetIndex && binding.Name == name;
        });
    }

    void ShaderBindings::Write(std::string_view name, std::span<VriDescriptor* const> descriptors, uint32_t first)
    {
        for (const auto& binding : m_Pipeline.GetBindings())
        {
            if (binding.PushConstant || binding.Set != m_SetIndex || binding.Name != name)
            {
                continue;
            }
            if (first + descriptors.size() > binding.Count)
            {
                throw std::out_of_range("Too many descriptors for " + binding.Name);
            }
            std::vector<const VriDescriptor*> list(descriptors.begin(), descriptors.end());
            VriDescriptorRangeUpdateDesc      update {list.data(), uint32_t(list.size()), first};
            m_Device.Core.UpdateDescriptorRanges(m_Set, m_Pipeline.RangeIndex(binding), 1, &update);
            return;
        }
    }

    void ShaderBindings::Bind(VriCommandBuffer* cmd) const
    {
        if (m_Set != nullptr)
        {
            m_Device.Core.CmdSetDescriptorSet(cmd, m_SetIndex, m_Set);
        }
    }

    // ---------------------------------------------------------------- ShaderMaterial

    ShaderMaterial::ShaderMaterial(RHI::Device& device, const ShaderAsset& shader, const ShaderPipeline& pipeline) :
        m_Device(device), m_Shader(shader), m_Pipeline(pipeline)
    {
        const auto set = pipeline.MaterialSet();
        if (!set)
        {
            return; // the pass does not read `material`
        }
        uint32_t textures = 0, samplers = 0, buffers = 0;
        for (const auto& binding : pipeline.GetBindings())
        {
            if (!binding.PushConstant && binding.Set == *set)
            {
                textures += binding.Type == VriDescriptorType_Texture ? binding.Count : 0;
                samplers += binding.Type == VriDescriptorType_Sampler ? binding.Count : 0;
                buffers += binding.Type == VriDescriptorType_ConstantBuffer ? binding.Count : 0;
            }
        }
        VriDescriptorPoolDesc pool {};
        pool.descriptorSetMaxNum  = 1;
        pool.textureMaxNum        = textures;
        pool.samplerMaxNum        = samplers;
        pool.constantBufferMaxNum = buffers;
        RHI::Check(device.Core.CreateDescriptorPool(device.Handle, &pool, &m_Pool), "Create material descriptor pool");
        RHI::Check(device.Core.AllocateDescriptorSets(m_Pool, pipeline.GetLayout(), *set, &m_Set, 1), "Allocate material descriptor set");

        if (const auto* layout = pipeline.MaterialLayout(); layout != nullptr && layout->Size != 0)
        {
            const uint64_t size = (layout->Size + 255) / 256 * 256;
            m_Uniforms.resize(size_t(size));
            m_UniformBuffer = CreateScope<RHI::Buffer>(device, VriBufferDesc {size, 0, VriBufferUsage_ConstantBuffer, VriMemoryLocation_HostUpload});
            VriBufferViewDesc view {};
            view.buffer   = m_UniformBuffer->Handle;
            view.viewType = VriDescriptorType_ConstantBuffer;
            view.size     = size;
            RHI::Check(device.Core.CreateBufferView(device.Handle, &view, &m_UniformView), "Create material buffer view");
            for (const auto& binding : pipeline.GetBindings())
            {
                if (binding.Name == "material" && binding.Type == VriDescriptorType_ConstantBuffer)
                {
                    const VriDescriptor*         descriptor = m_UniformView;
                    VriDescriptorRangeUpdateDesc update {&descriptor, 1, 0};
                    device.Core.UpdateDescriptorRanges(m_Set, pipeline.RangeIndex(binding), 1, &update);
                }
            }
        }

        VriSamplerDesc sampler {};
        sampler.magFilter    = VriFilter_Linear;
        sampler.minFilter    = VriFilter_Linear;
        sampler.mipmapMode   = VriMipmapMode_Linear;
        sampler.addressModeU = VriAddressMode_Repeat;
        sampler.addressModeV = VriAddressMode_Repeat;
        sampler.addressModeW = VriAddressMode_Repeat;
        sampler.maxLod       = 1000.0f;
        RHI::Check(device.Core.CreateSampler(device.Handle, &sampler, &m_Sampler), "Create material sampler");

        for (const auto& property : shader.Properties)
        {
            Write(property, property.DefaultValue);
        }
    }

    ShaderMaterial::~ShaderMaterial()
    {
        m_Device.WaitIdle();
        if (m_Pool != nullptr)
        {
            m_Device.Core.DestroyDescriptorPool(m_Pool);
        }
        if (m_UniformView != nullptr)
        {
            m_Device.Core.DestroyDescriptor(m_UniformView);
        }
        if (m_Sampler != nullptr)
        {
            m_Device.Core.DestroyDescriptor(m_Sampler);
        }
    }

    RHI::Texture& ShaderMaterial::Builtin(const std::string& name)
    {
        if (auto found = m_Builtins.find(name); found != m_Builtins.end())
        {
            return *found->second;
        }
        std::array<std::byte, 4> texel {};
        const std::array<uint8_t, 4> rgba = name == "black" ? std::array<uint8_t, 4> {0, 0, 0, 255} :
                                            name == "normal" ? std::array<uint8_t, 4> {128, 128, 255, 255} :
                                                               std::array<uint8_t, 4> {255, 255, 255, 255};
        std::memcpy(texel.data(), rgba.data(), 4);
        auto desc    = RHI::ColorTextureDesc({1, 1}, VriFormat_RGBA8_UNORM);
        desc.usage   = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
        auto texture = CreateScope<RHI::Texture>(m_Device, desc);
        RHI::UploadTexture2D(m_Device, *texture, texel);
        auto& result     = *texture;
        m_Builtins[name] = std::move(texture);
        return result;
    }

    void ShaderMaterial::WriteTexture(const ShaderProperty& property, VriDescriptor* view)
    {
        for (const auto& binding : m_Pipeline.GetBindings())
        {
            const VriDescriptor* descriptor = nullptr;
            if (binding.Name == "material." + property.Name)
            {
                descriptor = view;
            }
            else if (binding.Name == "material." + property.Name + "Sampler")
            {
                descriptor = m_Sampler;
            }
            if (descriptor != nullptr)
            {
                VriDescriptorRangeUpdateDesc update {&descriptor, 1, 0};
                m_Device.Core.UpdateDescriptorRanges(m_Set, m_Pipeline.RangeIndex(binding), 1, &update);
            }
        }
    }

    void ShaderMaterial::Write(const ShaderProperty& property, const ShaderPropertyValue& value)
    {
        if (m_Set == nullptr)
        {
            return;
        }
        if (const auto* texture = std::get_if<ShaderTextureValue>(&value))
        {
            if (property.Type != ShaderPropertyType::Texture2D)
            {
                throw std::invalid_argument("Only 2D texture properties are supported yet: " + property.Name);
            }
            WriteTexture(property, Builtin(texture->Builtin.empty() ? "white" : texture->Builtin).View());
            if (const auto* layout = m_Pipeline.MaterialLayout())
            {
                if (const auto* field = layout->Field(property.Name + "ScaleOffset"))
                {
                    std::memcpy(m_Uniforms.data() + field->Offset(ShaderOffsetKind::Uniform), texture->ScaleOffset.data(), 16);
                }
            }
            m_Dirty = true;
            return;
        }
        const auto* layout = m_Pipeline.MaterialLayout();
        const auto* field  = layout != nullptr ? layout->Field(property.Name) : nullptr;
        if (field == nullptr)
        {
            return; // compiled out: the pass does not use it
        }
        auto* destination = m_Uniforms.data() + field->Offset(ShaderOffsetKind::Uniform);
        if (const auto* scalar = std::get_if<float>(&value))
        {
            std::memcpy(destination, scalar, 4);
        }
        else if (const auto* integer = std::get_if<int32_t>(&value))
        {
            std::memcpy(destination, integer, 4);
        }
        else if (const auto* boolean = std::get_if<bool>(&value))
        {
            const uint32_t word = *boolean ? 1 : 0;
            std::memcpy(destination, &word, 4);
        }
        else if (const auto* vector = std::get_if<std::array<float, 4>>(&value))
        {
            std::memcpy(destination, vector->data(), 16);
        }
        m_Dirty = true;
    }

    void ShaderMaterial::Set(std::string_view name, const ShaderPropertyValue& value)
    {
        const auto& property = m_Shader.GetProperty(name);
        property.Validate(value);
        Write(property, value);
    }

    void ShaderMaterial::SetTexture(std::string_view name, RHI::Texture& texture)
    {
        const auto& property = m_Shader.GetProperty(name);
        if (property.Type != ShaderPropertyType::Texture2D)
        {
            throw std::invalid_argument("Only 2D texture properties are supported yet: " + property.Name);
        }
        if (m_Set != nullptr)
        {
            WriteTexture(property, texture.View());
        }
    }

    void ShaderMaterial::Bind(VriCommandBuffer* cmd)
    {
        if (m_Set == nullptr)
        {
            return;
        }
        if (m_Dirty && m_UniformBuffer)
        {
            // One frame in flight: the buffer is not in use by the GPU while recording.
            void* mapped = m_Device.Core.MapBuffer(m_UniformBuffer->Handle, 0, m_Uniforms.size());
            if (mapped == nullptr)
            {
                throw std::runtime_error("Map material buffer failed");
            }
            std::memcpy(mapped, m_Uniforms.data(), m_Uniforms.size());
            m_Device.Core.UnmapBuffer(m_UniformBuffer->Handle);
            m_Dirty = false;
        }
        m_Device.Core.CmdSetDescriptorSet(cmd, *m_Pipeline.MaterialSet(), m_Set);
    }
} // namespace SnowLeopardEngine
