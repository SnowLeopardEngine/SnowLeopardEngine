// DzShader v3: parser (ANTLR parse tree -> ShaderAsset), validation, generated Slang and compilation.
// Ported from libvultra (MIT): shader_parser.cpp, shader_property.cpp, shader_asset.cpp, shader_compiler.cpp.

#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderAsset.h"

#include "Grammar/generated/dzshader_lexer.h"
#include "Grammar/generated/dzshader_parser.h"

#include <cmath>
#include <fstream>
#include <limits>
#include <set>
#include <stdexcept>

namespace SnowLeopardEngine
{
    // ---------------------------------------------------------------- properties

    std::string ShaderSourceLocation::Describe() const
    {
        return File.generic_string() + ":" + std::to_string(Line) + ":" + std::to_string(Column);
    }

    bool ShaderProperty::IsTexture() const
    {
        return Type >= ShaderPropertyType::Texture2D && Type <= ShaderPropertyType::TextureCubeArray;
    }

    void ShaderProperty::Validate(const ShaderPropertyValue& value) const
    {
        auto fail = [&](const std::string& message) {
            throw std::invalid_argument(Location.Describe() + ": property " + Name + ": " + message);
        };
        size_t expected = 0; // variant index
        if (Type == ShaderPropertyType::Integer)
        {
            expected = 1;
        }
        else if (Type == ShaderPropertyType::Boolean)
        {
            expected = 2;
        }
        else if (Type == ShaderPropertyType::Vector || Type == ShaderPropertyType::Color)
        {
            expected = 3;
        }
        else if (IsTexture())
        {
            expected = 4;
        }
        if (value.index() != expected)
        {
            fail("value does not match the declared type");
        }
        if (const auto* scalar = std::get_if<float>(&value))
        {
            if (!std::isfinite(*scalar))
            {
                fail("value must be finite");
            }
            if (Type == ShaderPropertyType::Range && (*scalar < Minimum || *scalar > Maximum))
            {
                fail("value is outside its Range");
            }
        }
        if (const auto* vector = std::get_if<std::array<float, 4>>(&value))
        {
            for (const auto component : *vector)
            {
                if (!std::isfinite(component))
                {
                    fail("components must be finite");
                }
            }
        }
        if (const auto* integer = std::get_if<int32_t>(&value); integer != nullptr && !Enumeration.empty())
        {
            if (std::none_of(Enumeration.begin(), Enumeration.end(), [&](const auto& item) { return item.second == *integer; }))
            {
                fail("value is not one of its Enum values");
            }
        }
        if (const auto* texture = std::get_if<ShaderTextureValue>(&value))
        {
            if (!texture->Builtin.empty() && texture->Builtin != "white" && texture->Builtin != "black" &&
                texture->Builtin != "normal")
            {
                fail("unknown builtin texture " + texture->Builtin);
            }
        }
    }

    const ShaderProgram& ShaderPass::GetProgram(std::string_view variant) const
    {
        const auto found = Programs.find(variant);
        if (found == Programs.end())
        {
            throw std::invalid_argument("Pass " + Name + " has no compiled program for variant " + std::string(variant));
        }
        return found->second;
    }

    const ShaderPass* ShaderSubshader::FindPass(std::string_view name) const
    {
        for (const auto& pass : Passes)
        {
            if (pass.Name == name)
            {
                return &pass;
            }
        }
        return nullptr;
    }

    const ShaderProperty& ShaderAsset::GetProperty(std::string_view name) const
    {
        for (const auto& property : Properties)
        {
            if (property.Name == name)
            {
                return property;
            }
        }
        throw std::invalid_argument("Shader " + Name + " has no property " + std::string(name));
    }

    const ShaderVariant& ShaderAsset::GetVariant(std::string_view name) const
    {
        for (const auto& variant : Variants)
        {
            if (variant.Name == name)
            {
                return variant;
            }
        }
        throw std::invalid_argument("Shader " + Name + " has no variant " + std::string(name));
    }

    // ---------------------------------------------------------------- parser

    namespace
    {
        using Parser  = dzshader_parser;
        using Context = antlr4::ParserRuleContext;

        ShaderSourceLocation LocationOf(const std::filesystem::path& file, const antlr4::Token* token)
        {
            return {file, uint32_t(token->getLine()), uint32_t(token->getCharPositionInLine() + 1)};
        }

        [[noreturn]] void Fail(const std::filesystem::path& file, const Context* context, const std::string& message)
        {
            throw std::invalid_argument(LocationOf(file, context->start).Describe() + ": " + message);
        }

        class ErrorListener final : public antlr4::BaseErrorListener
        {
        public:
            explicit ErrorListener(std::filesystem::path file) : m_File(std::move(file)) {}

            void syntaxError(antlr4::Recognizer*, antlr4::Token*, size_t line, size_t column, const std::string& message, std::exception_ptr) override
            {
                m_Errors += ShaderSourceLocation {m_File, uint32_t(line), uint32_t(column + 1)}.Describe() + ": " + message + "\n";
            }

            const std::string& Errors() const { return m_Errors; }

        private:
            std::filesystem::path m_File;
            std::string           m_Errors;
        };

        // A grammar STRING token ("..." with backslash escapes) to its value.
        std::string Unquote(antlr4::tree::TerminalNode* node)
        {
            const auto  text = node->getText();
            std::string result;
            for (size_t i = 1; i + 1 < text.size(); ++i)
            {
                char c = text[i];
                if (c == '\\' && i + 2 < text.size())
                {
                    c = text[++i];
                    switch (c)
                    {
                        case 'n':
                            c = '\n';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        default:
                            break; // \" \\ and anything else: the character itself
                    }
                }
                result += c;
            }
            return result;
        }

        double Number(const std::filesystem::path& file, Context* context, const std::string& text)
        {
            size_t end    = 0;
            double result = 0;
            try
            {
                result = std::stod(text, &end);
            }
            catch (const std::exception&)
            {
                Fail(file, context, "expected a representable number");
            }
            if (!std::isfinite(result) || end != text.size())
            {
                Fail(file, context, "expected a finite number");
            }
            return result;
        }

        int32_t Integer(const std::filesystem::path& file, Context* context, double value)
        {
            if (std::trunc(value) != value || value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max())
            {
                Fail(file, context, "expected a signed 32-bit integer");
            }
            return int32_t(value);
        }

        ShaderProperty ReadProperty(const std::filesystem::path& file, Parser::PropertyContext* context)
        {
            ShaderProperty property;
            property.Name     = context->IDENTIFIER()->getText();
            property.Label    = Unquote(context->STRING());
            property.Location = LocationOf(file, context->IDENTIFIER()->getSymbol());

            auto*      type     = context->propertyType();
            const auto typeName = type->IDENTIFIER() != nullptr ? type->IDENTIFIER()->getText() : type->TEXTURE_TYPE()->getText();
            static const std::map<std::string, ShaderPropertyType> types {{"Float", ShaderPropertyType::Float},
                                                                          {"Range", ShaderPropertyType::Range},
                                                                          {"Integer", ShaderPropertyType::Integer},
                                                                          {"Boolean", ShaderPropertyType::Boolean},
                                                                          {"Vector", ShaderPropertyType::Vector},
                                                                          {"Color", ShaderPropertyType::Color},
                                                                          {"2D", ShaderPropertyType::Texture2D},
                                                                          {"2DArray", ShaderPropertyType::Texture2DArray},
                                                                          {"3D", ShaderPropertyType::Texture3D},
                                                                          {"Cube", ShaderPropertyType::TextureCube},
                                                                          {"CubeArray", ShaderPropertyType::TextureCubeArray}};
            if (!types.contains(typeName))
            {
                Fail(file, type, "unknown property type " + typeName);
            }
            property.Type = types.at(typeName);
            if (property.Type == ShaderPropertyType::Range)
            {
                if (type->NUMBER().size() != 2)
                {
                    Fail(file, type, "Range needs a minimum and a maximum");
                }
                property.Minimum = Number(file, type, type->NUMBER(0)->getText());
                property.Maximum = Number(file, type, type->NUMBER(1)->getText());
                if (property.Minimum >= property.Maximum)
                {
                    Fail(file, type, "Range minimum must be below its maximum");
                }
            }
            else if (!type->NUMBER().empty())
            {
                Fail(file, type, "only Range takes numeric type arguments");
            }

            auto* value = context->value();
            if (property.IsTexture())
            {
                if (value->STRING() == nullptr)
                {
                    Fail(file, value, "texture defaults are a quoted builtin name (\"white\", \"black\", \"normal\") or \"\"");
                }
                ShaderTextureValue texture;
                texture.Builtin       = Unquote(value->STRING());
                property.DefaultValue = std::move(texture);
            }
            else if (property.Type == ShaderPropertyType::Vector || property.Type == ShaderPropertyType::Color)
            {
                if (value->LPAREN() == nullptr || value->NUMBER().size() != 4)
                {
                    Fail(file, value, "Vector and Color defaults have four components");
                }
                std::array<float, 4> components {};
                for (size_t index = 0; index < components.size(); ++index)
                {
                    components[index] = float(Number(file, value, value->NUMBER(index)->getText()));
                }
                property.DefaultValue = components;
            }
            else if (property.Type == ShaderPropertyType::Boolean)
            {
                if (value->IDENTIFIER() == nullptr || (value->getText() != "true" && value->getText() != "false"))
                {
                    Fail(file, value, "Boolean defaults are true or false");
                }
                property.DefaultValue = value->getText() == "true";
            }
            else
            {
                if (value->LPAREN() != nullptr || value->NUMBER().size() != 1)
                {
                    Fail(file, value, "expected one number");
                }
                const auto scalar = Number(file, value, value->NUMBER(0)->getText());
                if (property.Type == ShaderPropertyType::Integer)
                {
                    property.DefaultValue = Integer(file, value, scalar);
                }
                else
                {
                    property.DefaultValue = float(scalar);
                }
            }

            std::set<std::string> attributes;
            for (auto* attribute : context->attribute())
            {
                const auto attributeName = attribute->IDENTIFIER()->getText();
                if (!attributes.insert(attributeName).second)
                {
                    Fail(file, attribute, "duplicate property attribute " + attributeName);
                }
                if (attributeName == "HDR")
                {
                    property.HDR = true;
                }
                else if (attributeName == "Normal")
                {
                    property.Normal = true;
                }
                else if (attributeName == "SRGB")
                {
                    property.SRGB = true;
                }
                else if (attributeName == "Linear")
                {
                    property.SRGB = false;
                }
                else if (attributeName == "HideInInspector")
                {
                    property.Hidden = true;
                }
                else if (attributeName == "NoScaleOffset")
                {
                    property.NoScaleOffset = true;
                }
                else if (attributeName == "Enum")
                {
                    const auto arguments = attribute->attributeArgument();
                    if (arguments.empty() || arguments.size() % 2 != 0)
                    {
                        Fail(file, attribute, "Enum takes label / integer pairs");
                    }
                    std::set<std::string> labels;
                    std::set<int32_t>     values;
                    for (size_t index = 0; index < arguments.size(); index += 2)
                    {
                        if ((arguments[index]->IDENTIFIER() == nullptr && arguments[index]->STRING() == nullptr) ||
                            arguments[index + 1]->NUMBER() == nullptr)
                        {
                            Fail(file, attribute, "Enum takes label / integer pairs");
                        }
                        const auto label = arguments[index]->STRING() != nullptr ? Unquote(arguments[index]->STRING()) :
                                                                                   arguments[index]->getText();
                        const auto enumValue = Integer(file, attribute, Number(file, attribute, arguments[index + 1]->getText()));
                        if (!labels.insert(label).second || !values.insert(enumValue).second)
                        {
                            Fail(file, attribute, "duplicate Enum label or value");
                        }
                        property.Enumeration.emplace_back(label, enumValue);
                    }
                }
                else
                {
                    Fail(file, attribute, "unknown property attribute " + attributeName);
                }
                if (attributeName != "Enum" && !attribute->attributeArgument().empty())
                {
                    Fail(file, attribute, attributeName + " takes no arguments");
                }
            }
            if ((property.HDR && property.Type != ShaderPropertyType::Color) ||
                ((property.Normal || property.NoScaleOffset || attributes.contains("SRGB") || attributes.contains("Linear")) &&
                 !property.IsTexture()) ||
                (property.Normal && property.SRGB) || (attributes.contains("SRGB") && attributes.contains("Linear")) ||
                (!property.Enumeration.empty() && property.Type != ShaderPropertyType::Integer))
            {
                Fail(file, context, "property attributes do not match its type");
            }
            property.Validate(property.DefaultValue);
            return property;
        }

        ShaderState ReadState(const std::filesystem::path& file, const std::vector<Parser::StateContext*>& contexts)
        {
            ShaderState state;
            state.Location.File = file;
            for (auto* context : contexts)
            {
                state.Location = LocationOf(file, context->start);
                auto add       = [&](const std::string& name, std::vector<ShaderStateValue> values) {
                    if (!state.Commands.emplace(name, std::move(values)).second)
                    {
                        Fail(file, context, "duplicate render state " + name);
                    }
                };
                if (context->STENCIL() != nullptr)
                {
                    add("Stencil", {{"On", false}});
                    const auto names = context->IDENTIFIER();
                    for (size_t index = 0; index < names.size(); ++index)
                    {
                        auto* value = context->stateValue(index);
                        add("Stencil." + names[index]->getText(),
                            {{value->LBRACKET() != nullptr ? value->IDENTIFIER()->getText() : value->getText(), value->LBRACKET() != nullptr}});
                    }
                }
                else
                {
                    std::vector<ShaderStateValue> values;
                    for (auto* value : context->stateValue())
                    {
                        values.push_back({value->LBRACKET() != nullptr ? value->IDENTIFIER()->getText() : value->getText(),
                                          value->LBRACKET() != nullptr});
                    }
                    if (context->NUMBER() != nullptr)
                    {
                        values.push_back({context->NUMBER()->getText(), false});
                    }
                    add(context->start->getText(), std::move(values));
                }
            }
            return state;
        }

        std::map<std::string, std::string, std::less<>> ReadTags(const std::filesystem::path& file, const std::vector<Parser::TagsContext*>& contexts)
        {
            std::map<std::string, std::string, std::less<>> tags;
            for (auto* context : contexts)
            {
                const auto values = context->STRING();
                for (size_t index = 0; index + 1 < values.size(); index += 2)
                {
                    const auto key = Unquote(values[index]);
                    if (!tags.emplace(key, Unquote(values[index + 1])).second)
                    {
                        Fail(file, context, "duplicate tag " + key);
                    }
                }
            }
            return tags;
        }

        template<class Program>
        ShaderSourceBlock ReadSource(const std::filesystem::path& file, Program* context)
        {
            const auto parts = context->PROGRAM_TEXT();
            if (parts.empty())
            {
                Fail(file, context, "Slang program is empty");
            }
            ShaderSourceBlock block;
            block.Location = LocationOf(file, parts.front()->getSymbol());
            for (auto* part : parts)
            {
                block.Text += part->getText();
            }
            return block;
        }

        ShaderVariant ReadVariant(const std::filesystem::path& file, Parser::VariantContext* context)
        {
            ShaderVariant variant;
            variant.Name     = Unquote(context->STRING());
            variant.Location = LocationOf(file, context->start);
            if (context->constants().size() > 1 || context->modules().size() > 1 || context->defines().size() > 1)
            {
                Fail(file, context, "duplicate variant section");
            }
            if (!context->modules().empty())
            {
                Fail(file, context, "variant Modules are not supported yet");
            }
            std::set<std::string> constants;
            for (auto* section : context->constants())
            {
                // children: Constants { (type name = value)* }
                for (size_t index = 2; index + 3 < section->children.size() - 1; index += 4)
                {
                    const auto type  = section->children[index]->getText();
                    const auto name  = section->children[index + 1]->getText();
                    const auto value = section->children[index + 3]->getText();
                    if ((type != "bool" && type != "int" && type != "uint" && type != "float") || !constants.insert(name).second)
                    {
                        Fail(file, section, "invalid or duplicate link-time constant " + name);
                    }
                    if (type == "bool" ? (value != "true" && value != "false") : false)
                    {
                        Fail(file, section, "bool constants are true or false");
                    }
                    if (type != "bool")
                    {
                        const auto numeric = Number(file, section, value);
                        if (type == "int")
                        {
                            Integer(file, section, numeric);
                        }
                        if (type == "uint" && (std::trunc(numeric) != numeric || numeric < 0 || numeric > UINT32_MAX))
                        {
                            Fail(file, section, "expected an unsigned 32-bit constant");
                        }
                    }
                    variant.Constants += "export static const " + type + " " + name + " = " + value + ";\n";
                }
            }
            std::set<std::string> defines;
            for (auto* section : context->defines())
            {
                // children: Defines { (name = value)* }
                for (size_t index = 2; index + 2 < section->children.size() - 1; index += 3)
                {
                    const auto name  = section->children[index]->getText();
                    auto*      value = dynamic_cast<antlr4::tree::TerminalNode*>(section->children[index + 2]);
                    if (!defines.insert(name).second)
                    {
                        Fail(file, section, "duplicate variant define " + name);
                    }
                    variant.Defines.push_back({name, value->getSymbol()->getType() == Parser::STRING ? Unquote(value) : value->getText()});
                }
            }
            return variant;
        }

        ShaderPass ReadPass(const std::filesystem::path& file, Parser::PassContext* context)
        {
            ShaderPass pass;
            pass.Location = LocationOf(file, context->start);
            if (context->STRING().size() != 1 || context->program().size() != 1)
            {
                Fail(file, context, "a Pass needs exactly one Name and one SLANGPROGRAM");
            }
            if (!context->requires_().empty())
            {
                Fail(file, context, "Requires (mesh shaders, ray queries) is not supported yet");
            }
            pass.Name = Unquote(context->STRING(0));
            pass.Tags = ReadTags(file, context->tags());
            if (const auto found = pass.Tags.find("LightMode"); found != pass.Tags.end())
            {
                pass.LightMode = found->second;
            }
            pass.State  = ReadState(file, context->state());
            pass.Source = ReadSource(file, context->program(0));
            for (auto* stage : context->stage())
            {
                const auto         type = stage->start->getType();
                VriShaderStageBits bit  = VriShaderStage_Compute;
                if (type == Parser::VERTEX)
                {
                    bit = VriShaderStage_Vertex;
                }
                else if (type == Parser::FRAGMENT)
                {
                    bit = VriShaderStage_Fragment;
                }
                else if (type == Parser::TASK || type == Parser::MESH)
                {
                    Fail(file, stage, "mesh shading stages are not supported yet");
                }
                if (std::any_of(pass.Entries.begin(), pass.Entries.end(), [bit](const ShaderEntry& e) { return e.Stage == bit; }))
                {
                    Fail(file, stage, "duplicate stage in pass " + pass.Name);
                }
                pass.Entries.push_back({stage->IDENTIFIER()->getText(), bit});
            }
            const bool compute = std::any_of(pass.Entries.begin(), pass.Entries.end(), [](const auto& e) { return e.Stage == VriShaderStage_Compute; });
            const bool vertex  = std::any_of(pass.Entries.begin(), pass.Entries.end(), [](const auto& e) { return e.Stage == VriShaderStage_Vertex; });
            if (compute ? pass.Entries.size() != 1 : !vertex)
            {
                Fail(file, context, "a Pass is either one Compute entry or a Vertex (+ Fragment) pair");
            }
            return pass;
        }

        bool Identifier(std::string_view text)
        {
            if (text.empty() || !(std::isalpha(static_cast<unsigned char>(text[0])) || text[0] == '_'))
            {
                return false;
            }
            return std::all_of(text.begin(), text.end(), [](char c) { return std::isalnum(static_cast<unsigned char>(c)) || c == '_'; });
        }

        void ValidateState(const ShaderAsset& asset, const ShaderState& state)
        {
            static const std::vector<std::string> compare {"Never", "Less", "Equal", "LEqual", "Greater", "NotEqual", "GEqual", "Always"};
            static const std::vector<std::string> stencilOps {"Keep", "Zero", "Replace", "IncrSat", "DecrSat", "Invert", "IncrWrap", "DecrWrap"};
            static const std::map<std::string, std::vector<std::string>, std::less<>> enums {
                {"Cull", {"Off", "Front", "Back"}},
                {"FrontFace", {"CW", "CCW"}},
                {"ZWrite", {"On", "Off"}},
                {"ZTest", compare},
                {"Blend",
                 {"Off", "Zero", "One", "SrcColor", "OneMinusSrcColor", "DstColor", "OneMinusDstColor", "SrcAlpha", "OneMinusSrcAlpha",
                  "DstAlpha", "OneMinusDstAlpha", "SrcAlphaSaturate", "ConstantColor", "OneMinusConstantColor", "ConstantAlpha",
                  "OneMinusConstantAlpha"}},
                {"BlendOp", {"Add", "Subtract", "ReverseSubtract", "Min", "Max"}},
                {"Stencil", {"On", "Off"}},
                {"Stencil.Comp", compare},
                {"Stencil.Pass", stencilOps},
                {"Stencil.Fail", stencilOps},
                {"Stencil.ZFail", stencilOps}};
            auto fail = [&](const std::string& message) { throw std::invalid_argument(state.Location.Describe() + ": " + message); };
            for (const auto& [command, values] : state.Commands)
            {
                const bool numeric = command == "DepthBias" || command == "Stencil.Ref" || command == "Stencil.ReadMask" ||
                                     command == "Stencil.WriteMask";
                const bool mask = command == "ColorMask";
                if (!numeric && !mask && !enums.contains(command))
                {
                    fail("unknown render state " + command);
                }
                if (command == "Blend" && values.size() != 1 && values.size() != 2 && values.size() != 4)
                {
                    fail("Blend is Off, or two / four factors");
                }
                if (command == "BlendOp" && values.size() != 1 && values.size() != 2)
                {
                    fail("BlendOp is one colour operation and optionally one alpha operation");
                }
                for (const auto& value : values)
                {
                    if (value.Property)
                    {
                        // [name]: an Integer property, whose value selects the state (Enum values = state index).
                        const auto& property = asset.GetProperty(value.Value);
                        if (property.Type != ShaderPropertyType::Integer && property.Type != ShaderPropertyType::Float &&
                            property.Type != ShaderPropertyType::Range)
                        {
                            fail("render state " + command + " can only take a numeric property");
                        }
                        continue;
                    }
                    if (numeric)
                    {
                        try
                        {
                            (void)std::stod(value.Value);
                        }
                        catch (const std::exception&)
                        {
                            fail(command + " takes numbers");
                        }
                    }
                    else if (mask)
                    {
                        if (value.Value != "0" && value.Value.find_first_not_of("RGBA") != std::string::npos)
                        {
                            fail("ColorMask is a combination of R, G, B, A or 0");
                        }
                    }
                    else if (const auto& allowed = enums.at(command); std::find(allowed.begin(), allowed.end(), value.Value) == allowed.end())
                    {
                        fail("invalid value " + value.Value + " for " + command);
                    }
                }
            }
        }

        void Validate(const ShaderAsset& asset)
        {
            if (asset.Name.empty() || asset.Subshaders.empty())
            {
                throw std::invalid_argument(asset.SourcePath.generic_string() + ": a shader needs a name and a SubShader");
            }
            std::set<std::string> names;
            for (const auto& property : asset.Properties)
            {
                if (!Identifier(property.Name) || property.Name.starts_with("__dz") || !names.insert(property.Name).second)
                {
                    throw std::invalid_argument(property.Location.Describe() + ": invalid or duplicate property " + property.Name);
                }
            }
            for (const auto& property : asset.Properties)
            {
                if (property.IsTexture())
                {
                    for (const auto* suffix : {"Sampler", "ScaleOffset"})
                    {
                        if (names.contains(property.Name + suffix))
                        {
                            throw std::invalid_argument(property.Location.Describe() + ": property clashes with the generated " +
                                                        property.Name + suffix);
                        }
                    }
                }
            }
            std::set<std::string> variants;
            for (const auto& variant : asset.Variants)
            {
                if (variant.Name.empty() || !variants.insert(variant.Name).second)
                {
                    throw std::invalid_argument(variant.Location.Describe() + ": invalid or duplicate variant " + variant.Name);
                }
            }
            for (const auto& subshader : asset.Subshaders)
            {
                ValidateState(asset, subshader.State);
                if (!subshader.Surface && subshader.Passes.empty())
                {
                    throw std::invalid_argument(subshader.Location.Describe() + ": a SubShader needs a Surface or a Pass");
                }
                std::set<std::string> passNames;
                for (const auto& pass : subshader.Passes)
                {
                    ValidateState(asset, pass.State);
                    if (pass.Name.empty() || !passNames.insert(pass.Name).second)
                    {
                        throw std::invalid_argument(pass.Location.Describe() + ": invalid or duplicate pass name " + pass.Name);
                    }
                }
            }
        }

        // `#line` directive naming a file for Slang diagnostics.
        std::string LineDirective(uint32_t line, const std::filesystem::path& file)
        {
            std::string name = file.generic_string();
            std::string quoted;
            for (const char c : name)
            {
                if (c == '"' || c == '\\')
                {
                    quoted += '\\';
                }
                quoted += c;
            }
            return "#line " + std::to_string(line) + " \"" + quoted + "\"\n";
        }

        std::string SourceBlock(const ShaderSourceBlock& block)
        {
            return LineDirective(block.Location.Line, block.Location.File) + block.Text + "\n#line 1 \"dz_generated\"\n";
        }

        ShaderState InheritState(const ShaderState& inherited, const ShaderState& overrides)
        {
            auto result = inherited;
            if (!overrides.Commands.empty())
            {
                result.Location = overrides.Location;
            }
            for (const auto& [name, value] : overrides.Commands)
            {
                result.Commands.insert_or_assign(name, value);
            }
            return result;
        }
    } // namespace

    ShaderAsset ShaderAsset::Parse(const std::filesystem::path& file, std::string_view text)
    {
        if (file.extension() != ".dzshader" || text.size() > 16 * 1024 * 1024)
        {
            throw std::invalid_argument("Expected a .dzshader source of at most 16 MiB: " + file.generic_string());
        }
        const auto               source = std::filesystem::absolute(file).lexically_normal();
        antlr4::ANTLRInputStream input {std::string(text)};
        dzshader_lexer           lexer(&input);
        ErrorListener            errors(source);
        lexer.removeErrorListeners();
        lexer.addErrorListener(&errors);
        antlr4::CommonTokenStream tokens(&lexer);
        Parser                    parser(&tokens);
        parser.removeErrorListeners();
        parser.addErrorListener(&errors);
        auto* root = parser.file();
        if (!errors.Errors().empty())
        {
            throw std::invalid_argument(errors.Errors());
        }

        ShaderAsset asset;
        asset.Name       = Unquote(root->STRING());
        asset.SourcePath = source;
        if (root->properties().size() > 1)
        {
            Fail(source, root, "a shader has at most one Properties block");
        }
        for (auto* section : root->properties())
        {
            for (auto* property : section->property())
            {
                asset.Properties.push_back(ReadProperty(source, property));
            }
        }
        for (auto* variant : root->variant())
        {
            asset.Variants.push_back(ReadVariant(source, variant));
        }
        if (asset.Variants.empty())
        {
            asset.Variants.push_back({"Default", {}, {}, {source, 1, 1}});
        }
        for (auto* context : root->subshader())
        {
            ShaderSubshader subshader;
            subshader.Location = LocationOf(source, context->start);
            subshader.Tags     = ReadTags(source, context->tags());
            subshader.State    = ReadState(source, context->state());
            if (!context->requires_().empty())
            {
                Fail(source, context, "Requires (mesh shaders, ray queries) is not supported yet");
            }
            for (auto* common : context->common())
            {
                subshader.Common.push_back(ReadSource(source, common));
            }
            if (context->surface().size() > 1)
            {
                Fail(source, context, "a SubShader has at most one Surface");
            }
            if (!context->surface().empty())
            {
                auto* surface = context->surface(0);
                if (surface->MODEL().size() != 1 || surface->ENTRY().size() != 1 || surface->program().size() != 1)
                {
                    Fail(source, surface, "a Surface needs one Model, one Entry and one SLANGPROGRAM");
                }
                ShaderSurface result;
                result.Location = LocationOf(source, surface->start);
                for (size_t index = 0; index + 1 < surface->children.size(); ++index)
                {
                    if (surface->children[index]->getText() == "Model")
                    {
                        result.Model = surface->children[index + 1]->getText();
                    }
                    if (surface->children[index]->getText() == "Entry")
                    {
                        result.Entry = surface->children[index + 1]->getText();
                    }
                }
                result.State      = ReadState(source, surface->state());
                result.Source     = ReadSource(source, surface->program(0));
                subshader.Surface = std::move(result);
            }
            for (auto* pass : context->pass())
            {
                subshader.Passes.push_back(ReadPass(source, pass));
            }
            // Graphics passes start from the SubShader's render state.
            for (auto& pass : subshader.Passes)
            {
                if (pass.Entries.front().Stage != VriShaderStage_Compute)
                {
                    pass.State = InheritState(subshader.State, pass.State);
                }
            }
            asset.Subshaders.push_back(std::move(subshader));
        }
        Validate(asset);
        return asset;
    }

    std::string ShaderAsset::GeneratedMaterialSource() const
    {
        std::string result = "#line 1 \"dz_generated_material\"\nstruct DzMaterialProperties\n{\n";
        if (Properties.empty())
        {
            result += "    uint __dzEmpty;\n";
        }
        static constexpr std::array textureTypes {
            "Texture2D<float4>", "Texture2DArray<float4>", "Texture3D<float4>", "TextureCube<float4>", "TextureCubeArray<float4>"};
        for (const auto& property : Properties)
        {
            result += LineDirective(property.Location.Line, property.Location.File);
            std::string type = "float";
            if (property.Type == ShaderPropertyType::Integer)
            {
                type = "int";
            }
            else if (property.Type == ShaderPropertyType::Boolean)
            {
                type = "bool";
            }
            else if (property.Type == ShaderPropertyType::Vector || property.Type == ShaderPropertyType::Color)
            {
                type = "float4";
            }
            else if (property.IsTexture())
            {
                type = textureTypes[size_t(property.Type) - size_t(ShaderPropertyType::Texture2D)];
            }
            result += "    " + type + " " + property.Name + ";\n";
            if (property.IsTexture())
            {
                result += "    SamplerState " + property.Name + "Sampler;\n";
                if (!property.NoScaleOffset &&
                    (property.Type == ShaderPropertyType::Texture2D || property.Type == ShaderPropertyType::Texture2DArray))
                {
                    result += "    float4 " + property.Name + "ScaleOffset;\n";
                }
            }
        }
        result += "#line 1 \"dz_generated_material\"\n};\nParameterBlock<DzMaterialProperties> material;\n";
        return result;
    }

    std::string ShaderAsset::GeneratedPassSource(uint32_t subshaderIndex, uint32_t passIndex) const
    {
        const auto& subshader = Subshaders.at(subshaderIndex);
        const auto& pass      = subshader.Passes.at(passIndex);
        std::string source    = GeneratedMaterialSource();
        for (const auto& block : subshader.Common)
        {
            source += SourceBlock(block);
        }
        source += SourceBlock(pass.Source);
        return source;
    }

    ShaderAsset ShaderAsset::Load(const std::filesystem::path& file, const ShaderCompileOptions& options)
    {
        std::ifstream input(file, std::ios::binary);
        if (!input)
        {
            throw std::runtime_error("Cannot open shader: " + file.generic_string());
        }
        const std::string text {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
        return LoadSource(file, text, options);
    }

    ShaderAsset ShaderAsset::LoadSource(const std::filesystem::path& file, std::string_view text, const ShaderCompileOptions& options)
    {
        auto asset = Parse(file, text);
        if (!options.Entries.empty())
        {
            throw std::invalid_argument("DzShader entries come from its Passes, not from compile options");
        }
        for (const auto& subshader : asset.Subshaders)
        {
            if (subshader.Surface)
            {
                throw std::invalid_argument(subshader.Surface->Location.Describe() +
                                            ": Surface shaders compile once the lit pipeline exists (Phase 2.4)");
            }
        }
        ShaderCompiler compiler;
        for (uint32_t subshaderIndex = 0; subshaderIndex < asset.Subshaders.size(); ++subshaderIndex)
        {
            auto& subshader = asset.Subshaders[subshaderIndex];
            for (uint32_t passIndex = 0; passIndex < subshader.Passes.size(); ++passIndex)
            {
                auto&      pass   = subshader.Passes[passIndex];
                const auto source = asset.GeneratedPassSource(subshaderIndex, passIndex);
                for (const auto& variant : asset.Variants)
                {
                    auto settings    = options;
                    settings.Entries = pass.Entries;
                    settings.Defines.insert(settings.Defines.end(), variant.Defines.begin(), variant.Defines.end());
                    if (!variant.Constants.empty())
                    {
                        settings.LinkSources.push_back({"dz_variant_constants", variant.Constants});
                    }
                    try
                    {
                        auto program = compiler.CompileSource(asset.SourcePath, source, settings);
                        asset.Diagnostics += program.Diagnostics;
                        pass.Programs.emplace(variant.Name, std::move(program));
                    }
                    catch (const std::exception& error)
                    {
                        throw std::runtime_error(pass.Location.Describe() + ": pass " + pass.Name + ", variant " + variant.Name +
                                                 "\n" + error.what());
                    }
                }
            }
        }
        return asset;
    }
} // namespace SnowLeopardEngine
