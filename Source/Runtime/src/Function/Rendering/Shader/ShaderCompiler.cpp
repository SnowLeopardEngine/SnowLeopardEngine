#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderProgram.h"

#include <slang-com-ptr.h>
#include <slang.h>

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace SnowLeopardEngine
{
    namespace
    {
        VriShaderStageBits ToVriStage(SlangStage stage)
        {
            switch (stage)
            {
                case SLANG_STAGE_VERTEX:
                    return VriShaderStage_Vertex;
                case SLANG_STAGE_FRAGMENT:
                    return VriShaderStage_Fragment;
                case SLANG_STAGE_COMPUTE:
                    return VriShaderStage_Compute;
                case SLANG_STAGE_GEOMETRY:
                    return VriShaderStage_Geometry;
                case SLANG_STAGE_HULL:
                    return VriShaderStage_TessControl;
                case SLANG_STAGE_DOMAIN:
                    return VriShaderStage_TessEval;
                case SLANG_STAGE_AMPLIFICATION:
                    return VriShaderStage_Task;
                case SLANG_STAGE_MESH:
                    return VriShaderStage_Mesh;
                default:
                    throw std::invalid_argument("Unsupported shader stage");
            }
        }

        SlangStage ToSlangStage(VriShaderStageBits stage)
        {
            for (const auto candidate : {SLANG_STAGE_VERTEX,
                                         SLANG_STAGE_FRAGMENT,
                                         SLANG_STAGE_COMPUTE,
                                         SLANG_STAGE_GEOMETRY,
                                         SLANG_STAGE_HULL,
                                         SLANG_STAGE_DOMAIN,
                                         SLANG_STAGE_AMPLIFICATION,
                                         SLANG_STAGE_MESH})
            {
                if (ToVriStage(candidate) == stage)
                {
                    return candidate;
                }
            }
            throw std::invalid_argument("Unsupported shader stage");
        }

        uint32_t ColorOutputs(slang::VariableLayoutReflection* variable)
        {
            if (variable == nullptr)
            {
                return 0;
            }
            if (const auto* semantic = variable->getSemanticName())
            {
                std::string name = semantic;
                std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return char(std::tolower(c)); });
                if (name == "sv_target")
                {
                    return uint32_t(variable->getSemanticIndex()) + 1;
                }
            }
            uint32_t count = 0;
            auto*    type  = variable->getTypeLayout();
            for (uint32_t i = 0; i < type->getFieldCount(); ++i)
            {
                count = std::max(count, ColorOutputs(type->getFieldByIndex(i)));
            }
            return count;
        }

        // ---------------------------------------------------------------- reflection (libvultra shader_reflection)

        ShaderParameterKind ParameterKind(slang::TypeReflection::Kind kind)
        {
            using Kind = slang::TypeReflection::Kind;
            switch (kind)
            {
                case Kind::Scalar:
                    return ShaderParameterKind::Scalar;
                case Kind::Vector:
                    return ShaderParameterKind::Vector;
                case Kind::Matrix:
                    return ShaderParameterKind::Matrix;
                case Kind::Struct:
                    return ShaderParameterKind::Struct;
                case Kind::Array:
                    return ShaderParameterKind::Array;
                case Kind::ConstantBuffer:
                    return ShaderParameterKind::ConstantBuffer;
                case Kind::ParameterBlock:
                    return ShaderParameterKind::ParameterBlock;
                case Kind::Resource:
                    return ShaderParameterKind::Resource;
                case Kind::SamplerState:
                    return ShaderParameterKind::Sampler;
                default:
                    return ShaderParameterKind::None;
            }
        }

        ShaderScalarType ScalarType(slang::TypeReflection::ScalarType type)
        {
            using Type = slang::TypeReflection;
            switch (type)
            {
                case Type::None:
                    return ShaderScalarType::None;
                case Type::Float32:
                    return ShaderScalarType::Float;
                case Type::Int32:
                    return ShaderScalarType::Integer;
                case Type::UInt32:
                    return ShaderScalarType::Unsigned;
                case Type::Bool:
                    return ShaderScalarType::Boolean;
                default:
                    return ShaderScalarType::Other;
            }
        }

        void ReflectResource(slang::TypeLayoutReflection* type, ShaderParameter& result)
        {
            const auto shape    = type->getResourceShape();
            const auto base     = shape & SLANG_RESOURCE_BASE_SHAPE_MASK;
            const bool writable = type->getResourceAccess() != SLANG_RESOURCE_ACCESS_READ;
            if (base == SLANG_STRUCTURED_BUFFER || base == SLANG_BYTE_ADDRESS_BUFFER)
            {
                result.DescriptorType = writable ? VriDescriptorType_StorageBuffer : VriDescriptorType_StructuredBuffer;
            }
            else if (base >= SLANG_TEXTURE_1D && base <= SLANG_TEXTURE_CUBE)
            {
                result.DescriptorType = writable ? VriDescriptorType_StorageTexture : VriDescriptorType_Texture;
                const bool array      = (shape & SLANG_TEXTURE_ARRAY_FLAG) != 0;
                switch (base)
                {
                    case SLANG_TEXTURE_1D:
                        result.ViewType = array ? VriTextureViewType_1DArray : VriTextureViewType_1D;
                        break;
                    case SLANG_TEXTURE_2D:
                        result.ViewType = array ? VriTextureViewType_2DArray : VriTextureViewType_2D;
                        break;
                    case SLANG_TEXTURE_3D:
                        result.ViewType = VriTextureViewType_3D;
                        break;
                    case SLANG_TEXTURE_CUBE:
                        result.ViewType = array ? VriTextureViewType_CubeArray : VriTextureViewType_Cube;
                        break;
                    default:
                        break;
                }
            }
        }

        ShaderParameter ReflectType(slang::TypeLayoutReflection*     type,
                                    slang::VariableLayoutReflection* variable,
                                    std::string                      name,
                                    uint32_t                         depth)
        {
            if (type == nullptr || depth > 64)
            {
                throw std::invalid_argument("Invalid or too deeply nested Slang parameter layout");
            }
            ShaderParameter result;
            result.Name = std::move(name);
            if (const auto* typeName = type->getName())
            {
                result.TypeName = typeName;
            }
            result.Kind        = ParameterKind(type->getKind());
            result.Scalar      = ScalarType(type->getScalarType());
            result.Size        = type->getSize(SLANG_PARAMETER_CATEGORY_UNIFORM);
            result.Stride      = type->getStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
            result.Rows        = type->getRowCount();
            result.Columns     = type->getColumnCount();
            result.ColumnMajor = type->getMatrixLayoutMode() != SLANG_MATRIX_LAYOUT_ROW_MAJOR;
            if (result.Kind == ShaderParameterKind::Matrix)
            {
                // The layout includes each major vector's padding; split the reflected span evenly.
                const auto vectors = result.ColumnMajor ? result.Columns : result.Rows;
                if (vectors == 0 || result.Stride % vectors != 0)
                {
                    throw std::runtime_error("Slang matrix layout has no integral major-vector stride");
                }
                result.MatrixStride = result.Stride / vectors;
            }
            if (result.Kind == ShaderParameterKind::Resource)
            {
                ReflectResource(type, result);
                if ((type->getResourceShape() & SLANG_RESOURCE_BASE_SHAPE_MASK) == SLANG_STRUCTURED_BUFFER)
                {
                    result.Fields.push_back(ReflectType(type->getElementTypeLayout(), nullptr, "$element", depth + 1));
                }
            }
            constexpr std::array categories {
                std::pair {SLANG_PARAMETER_CATEGORY_UNIFORM, ShaderOffsetKind::Uniform},
                std::pair {SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT, ShaderOffsetKind::Descriptor},
                std::pair {SLANG_PARAMETER_CATEGORY_SUB_ELEMENT_REGISTER_SPACE, ShaderOffsetKind::RegisterSpace},
                std::pair {SLANG_PARAMETER_CATEGORY_PUSH_CONSTANT_BUFFER, ShaderOffsetKind::PushConstant}};
            for (uint32_t index = 0; index < type->getCategoryCount(); ++index)
            {
                for (const auto& [category, kind] : categories)
                {
                    if (uint32_t(type->getCategoryByIndex(index)) == uint32_t(category))
                    {
                        result.Offsets.push_back({kind,
                                                  variable != nullptr ? variable->getOffset(category) : 0,
                                                  variable != nullptr ? variable->getBindingSpace(category) : 0});
                    }
                }
            }
            if (result.Name == "$container")
            {
                return result;
            }
            if (result.Kind == ShaderParameterKind::Struct)
            {
                for (uint32_t index = 0; index < type->getFieldCount(); ++index)
                {
                    auto* field = type->getFieldByIndex(index);
                    result.Fields.push_back(ReflectType(field->getTypeLayout(), field, field->getName(), depth + 1));
                }
            }
            else if (result.Kind == ShaderParameterKind::Array)
            {
                result.Count  = type->getElementCount();
                result.Stride = type->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
                auto* element = type->getElementVarLayout();
                result.Fields.push_back(ReflectType(element != nullptr ? element->getTypeLayout() : type->getElementTypeLayout(),
                                                    element,
                                                    "$element",
                                                    depth + 1));
            }
            else if (result.Kind == ShaderParameterKind::ConstantBuffer || result.Kind == ShaderParameterKind::ParameterBlock)
            {
                if (auto* container = type->getContainerVarLayout())
                {
                    result.Fields.push_back(ReflectType(container->getTypeLayout(), container, "$container", depth + 1));
                }
                auto* element = type->getElementVarLayout();
                result.Fields.push_back(ReflectType(element != nullptr ? element->getTypeLayout() : type->getElementTypeLayout(),
                                                    element,
                                                    "$element",
                                                    depth + 1));
            }
            return result;
        }

        // ---------------------------------------------------------------- bindings (libvultra shader_bindings)

        bool HasOffset(const ShaderParameter& node, ShaderOffsetKind kind)
        {
            return std::any_of(node.Offsets.begin(), node.Offsets.end(), [kind](const auto& o) { return o.Kind == kind; });
        }

        void CollectBindings(const ShaderParameter&              node,
                             std::string                         name,
                             uint64_t                            set,
                             uint64_t                            binding,
                             uint64_t                            count,
                             std::vector<ShaderResourceBinding>& output)
        {
            if (node.Kind == ShaderParameterKind::ParameterBlock)
            {
                set += node.Offset(ShaderOffsetKind::RegisterSpace);
                binding = 0; // bindings are relative to the innermost parameter block
            }
            else
            {
                binding += node.Offset(ShaderOffsetKind::Descriptor);
                set += node.Space(ShaderOffsetKind::Descriptor);
            }
            if (node.Kind == ShaderParameterKind::ConstantBuffer || node.Kind == ShaderParameterKind::ParameterBlock)
            {
                const auto* element   = node.Field("$element");
                const auto* container = node.Field("$container");
                if (element == nullptr || container == nullptr)
                {
                    throw std::invalid_argument("Missing reflected container / element: " + name);
                }
                if (element->Size != 0)
                {
                    const bool push = HasOffset(node, ShaderOffsetKind::PushConstant) ||
                                      HasOffset(*container, ShaderOffsetKind::PushConstant);
                    output.push_back({name,
                                      VriDescriptorType_ConstantBuffer,
                                      uint32_t(set),
                                      uint32_t(binding + container->Offset(ShaderOffsetKind::Descriptor)),
                                      uint32_t(count),
                                      element->Size,
                                      VriTextureViewType_2D,
                                      push});
                }
                CollectBindings(*element, std::move(name), set, binding, count, output);
            }
            else if (node.Kind == ShaderParameterKind::Array)
            {
                if (node.Count == 0 || node.Count > 65536 || count > 65536 / node.Count || node.Fields.size() != 1)
                {
                    throw std::invalid_argument("Resource arrays must be bounded: " + name);
                }
                CollectBindings(node.Fields.front(), std::move(name), set, binding, count * node.Count, output);
            }
            else if (node.Kind == ShaderParameterKind::Struct)
            {
                for (const auto& field : node.Fields)
                {
                    auto child = name.empty() ? field.Name : name + "." + field.Name;
                    CollectBindings(field, std::move(child), set, binding, count, output);
                }
            }
            else if (node.Kind == ShaderParameterKind::Resource || node.Kind == ShaderParameterKind::Sampler)
            {
                VriDescriptorType type = VriDescriptorType_Sampler;
                if (node.Kind == ShaderParameterKind::Resource)
                {
                    if (!node.DescriptorType)
                    {
                        throw std::invalid_argument("Unsupported reflected resource: " + name + " (" + node.TypeName + ")");
                    }
                    type = *node.DescriptorType;
                }
                output.push_back({std::move(name), type, uint32_t(set), uint32_t(binding), uint32_t(count), 0, node.ViewType, false});
            }
        }
    } // namespace

    // ---------------------------------------------------------------- ShaderParameter / ShaderProgram

    uint64_t ShaderParameter::Offset(ShaderOffsetKind kind) const
    {
        for (const auto& offset : Offsets)
        {
            if (offset.Kind == kind)
            {
                return offset.Offset;
            }
        }
        return 0;
    }

    uint64_t ShaderParameter::Space(ShaderOffsetKind kind) const
    {
        for (const auto& offset : Offsets)
        {
            if (offset.Kind == kind)
            {
                return offset.Space;
            }
        }
        return 0;
    }

    const ShaderParameter* ShaderParameter::Field(std::string_view name) const
    {
        for (const auto& field : Fields)
        {
            if (field.Name == name)
            {
                return &field;
            }
        }
        return nullptr;
    }

    std::vector<VriShaderDesc> ShaderProgram::Descriptors(std::span<const ShaderEntry> entries) const
    {
        if (entries.empty())
        {
            throw std::invalid_argument("Select at least one shader entry");
        }
        std::vector<VriShaderDesc> result;
        for (const auto& entry : entries)
        {
            const auto found = std::find_if(Shaders.begin(), Shaders.end(), [&](const CompiledShader& shader) {
                return shader.Entry.Name == entry.Name && shader.Entry.Stage == entry.Stage;
            });
            if (found == Shaders.end())
            {
                throw std::invalid_argument("Shader program has no entry " + entry.Name + " for that stage");
            }
            VriShaderDesc desc {};
            desc.stage          = found->Entry.Stage;
            desc.bytecode       = found->Words.data();
            desc.bytecodeSize   = found->Words.size() * sizeof(uint32_t);
            desc.entryPointName = found->Entry.Name.c_str();
            result.push_back(desc);
        }
        return result;
    }

    std::vector<ShaderResourceBinding> ShaderProgram::ResourceBindings() const
    {
        std::vector<ShaderResourceBinding> result;
        CollectBindings(Parameters, "", 0, 0, 1, result);
        return result;
    }

    const CompiledShader* ShaderProgram::Find(VriShaderStageBits stage) const
    {
        for (const auto& shader : Shaders)
        {
            if (shader.Entry.Stage == stage)
            {
                return &shader;
            }
        }
        return nullptr;
    }

    // ---------------------------------------------------------------- ShaderCompiler

    struct ShaderCompiler::Impl
    {
        Slang::ComPtr<slang::IGlobalSession> Global;
    };

    ShaderCompiler::ShaderCompiler() : m_Impl(CreateScope<Impl>()) {}

    ShaderCompiler::~ShaderCompiler() = default;

    ShaderProgram ShaderCompiler::Compile(const std::filesystem::path& file, const ShaderCompileOptions& options)
    {
        std::ifstream input(file, std::ios::binary);
        if (!input)
        {
            throw std::runtime_error("Cannot open shader source: " + file.string());
        }
        const std::string text {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        return CompileSource(file, text, options);
    }

    ShaderProgram ShaderCompiler::CompileSource(const std::filesystem::path& sourcePath,
                                                std::string_view             sourceText,
                                                const ShaderCompileOptions&  options)
    {
        using Slang::ComPtr;
        ShaderProgram program;
        program.Profile = options.Profile;
        if (sourceText.find(char(0)) != std::string_view::npos)
        {
            throw std::invalid_argument("Shader source contains a NUL character: " + sourcePath.string());
        }

        const auto source   = std::filesystem::absolute(sourcePath).lexically_normal();
        auto       diagnose = [&](SlangResult result, const ComPtr<slang::IBlob>& blob, const char* operation) {
            if (blob)
            {
                program.Diagnostics += static_cast<const char*>(blob->getBufferPointer());
            }
            if (SLANG_FAILED(result))
            {
                throw std::runtime_error(source.string() + ": " + operation + "\n" + program.Diagnostics);
            }
        };

        auto& global = m_Impl->Global;
        if (!global)
        {
            diagnose(slang::createGlobalSession(global.writeRef()), {}, "Create Slang global session");
        }

        slang::TargetDesc target {};
        target.format  = SLANG_SPIRV;
        target.profile = global->findProfile(program.Profile.c_str());
        if (!program.Profile.starts_with("spirv_") || target.profile == SLANG_PROFILE_UNKNOWN)
        {
            throw std::invalid_argument("Unsupported SPIR-V profile: " + program.Profile);
        }

        std::vector<slang::CompilerOptionEntry> compilerOptions;
        for (const auto name : {slang::CompilerOptionName::VulkanUseEntryPointName, slang::CompilerOptionName::EmitSpirvDirectly})
        {
            slang::CompilerOptionEntry option {};
            option.name            = name;
            option.value.kind      = slang::CompilerOptionValueKind::Int;
            option.value.intValue0 = 1;
            compilerOptions.push_back(option);
        }
#ifndef NDEBUG
        slang::CompilerOptionEntry debugInfo {};
        debugInfo.name            = slang::CompilerOptionName::DebugInformation;
        debugInfo.value.kind      = slang::CompilerOptionValueKind::Int;
        debugInfo.value.intValue0 = SLANG_DEBUG_INFO_LEVEL_STANDARD;
        compilerOptions.push_back(debugInfo);
#endif

        std::vector<std::string> paths {source.parent_path().string()};
        for (const auto& directory : options.IncludeDirectories)
        {
            paths.push_back(std::filesystem::absolute(directory).lexically_normal().string());
        }
        std::vector<const char*> searchPaths;
        for (const auto& directory : paths)
        {
            searchPaths.push_back(directory.c_str());
        }
        std::vector<slang::PreprocessorMacroDesc> macros;
        for (const auto& define : options.Defines)
        {
            macros.push_back({define.Name.c_str(), define.Value.c_str()});
        }

        slang::SessionDesc desc {};
        desc.targets                  = &target;
        desc.targetCount              = 1;
        desc.searchPaths              = searchPaths.data();
        desc.searchPathCount          = SlangInt(searchPaths.size());
        desc.preprocessorMacros       = macros.data();
        desc.preprocessorMacroCount   = SlangInt(macros.size());
        desc.defaultMatrixLayoutMode  = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        desc.compilerOptionEntries    = compilerOptions.data();
        desc.compilerOptionEntryCount = uint32_t(compilerOptions.size());
        // One session per compile: link-time inputs cannot leak between programs.
        ComPtr<slang::ISession> session;
        diagnose(global->createSession(desc, session.writeRef()), {}, "Create Slang session");

        ComPtr<slang::IBlob> diagnostics;
        const std::string    sourceBuffer(sourceText);
        auto*                module = session->loadModuleFromSourceString(
            source.stem().string().c_str(), source.string().c_str(), sourceBuffer.c_str(), diagnostics.writeRef());
        diagnose(module != nullptr ? SLANG_OK : SLANG_FAIL, diagnostics, "Load shader module");

        std::vector<ComPtr<slang::IEntryPoint>> entries;
        if (options.Entries.empty())
        {
            entries.resize(size_t(module->getDefinedEntryPointCount()));
            for (size_t i = 0; i < entries.size(); ++i)
            {
                diagnose(module->getDefinedEntryPoint(SlangInt32(i), entries[i].writeRef()), {}, "Discover shader entry point");
            }
        }
        else
        {
            entries.resize(options.Entries.size());
            for (size_t i = 0; i < entries.size(); ++i)
            {
                diagnostics.setNull();
                diagnose(module->findAndCheckEntryPoint(options.Entries[i].Name.c_str(),
                                                        ToSlangStage(options.Entries[i].Stage),
                                                        entries[i].writeRef(),
                                                        diagnostics.writeRef()),
                         diagnostics,
                         ("Find shader entry point " + options.Entries[i].Name).c_str());
            }
        }
        if (entries.empty())
        {
            throw std::invalid_argument("No shader entry points: " + source.string());
        }

        std::vector<slang::IComponentType*> components {module};
        for (const auto& entry : entries)
        {
            components.push_back(entry);
        }
        for (const auto& linkSource : options.LinkSources)
        {
            diagnostics.setNull();
            const auto syntheticPath = source.parent_path() / (linkSource.Name + ".slang");
            auto*      linked        = session->loadModuleFromSourceString(
                linkSource.Name.c_str(), syntheticPath.string().c_str(), linkSource.Value.c_str(), diagnostics.writeRef());
            diagnose(linked != nullptr ? SLANG_OK : SLANG_FAIL, diagnostics, "Load link-time Slang source");
            components.push_back(linked);
        }

        ComPtr<slang::IComponentType> composed;
        ComPtr<slang::IComponentType> linked;
        diagnostics.setNull();
        diagnose(session->createCompositeComponentType(
                     components.data(), SlangInt(components.size()), composed.writeRef(), diagnostics.writeRef()),
                 diagnostics,
                 "Compose shader");
        diagnostics.setNull();
        diagnose(composed->link(linked.writeRef(), diagnostics.writeRef()), diagnostics, "Link shader");
        diagnostics.setNull();
        auto* layout = linked->getLayout(0, diagnostics.writeRef());
        diagnose(layout != nullptr ? SLANG_OK : SLANG_FAIL, diagnostics, "Reflect shader");

        for (size_t i = 0; i < entries.size(); ++i)
        {
            auto*       reflection = layout->getEntryPointByIndex(SlangUInt(i));
            ShaderEntry entry {reflection->getName(), ToVriStage(reflection->getStage())};
            ComPtr<slang::IBlob> code;
            diagnostics.setNull();
            diagnose(linked->getEntryPointCode(SlangInt(i), 0, code.writeRef(), diagnostics.writeRef()), diagnostics, "Emit SPIR-V");
            if (!code || code->getBufferSize() == 0 || code->getBufferSize() % sizeof(uint32_t) != 0)
            {
                throw std::runtime_error("Slang emitted no valid SPIR-V for " + entry.Name);
            }
            CompiledShader shader;
            shader.Entry = std::move(entry);
            shader.Words.resize(code->getBufferSize() / sizeof(uint32_t));
            std::memcpy(shader.Words.data(), code->getBufferPointer(), code->getBufferSize());
            if (shader.Entry.Stage == VriShaderStage_Fragment)
            {
                shader.ColorOutputs = ColorOutputs(reflection->getResultVarLayout());
                for (uint32_t parameter = 0; parameter < reflection->getParameterCount(); ++parameter)
                {
                    shader.ColorOutputs = std::max(shader.ColorOutputs, ColorOutputs(reflection->getParameterByIndex(parameter)));
                }
            }
            if (shader.Entry.Stage == VriShaderStage_Compute || shader.Entry.Stage == VriShaderStage_Task ||
                shader.Entry.Stage == VriShaderStage_Mesh)
            {
                SlangUInt sizes[3] {};
                reflection->getComputeThreadGroupSize(3, sizes);
                for (uint32_t axis = 0; axis < 3; ++axis)
                {
                    shader.ThreadGroup[axis] = uint32_t(sizes[axis]);
                }
            }
            program.Shaders.push_back(std::move(shader));
        }
        program.Parameters = ReflectType(layout->getGlobalParamsVarLayout()->getTypeLayout(),
                                         layout->getGlobalParamsVarLayout(),
                                         "$globals",
                                         0);
        return program;
    }
} // namespace SnowLeopardEngine
