#include "SnowLeopardEngine/Function/Rendering/Renderer/UIRenderer.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <algorithm>
#include <cmath>

namespace SnowLeopardEngine
{
    namespace
    {
        constexpr uint32_t kSdfFlag = 0x80000000u;

        uint32_t PackColor(const glm::vec4& color)
        {
            const auto c = glm::clamp(color, glm::vec4(0), glm::vec4(1)) * 255.0f + 0.5f;
            return uint32_t(c.r) | (uint32_t(c.g) << 8) | (uint32_t(c.b) << 16) | (uint32_t(c.a) << 24);
        }

        const VriVertexAttributeDesc kAttributes[] {{VriFormat_RG32_SFLOAT, 0, 0, "POSITION", 0},
                                                    {VriFormat_RG32_SFLOAT, 8, 0, "TEXCOORD", 0},
                                                    {VriFormat_RGBA8_UNORM, 16, 0, "COLOR", 0},
                                                    {VriFormat_R32_UINT, 20, 0, "TEXTURE", 0}};
        const VriVertexStreamDesc    kStream[] {{uint32_t(sizeof(UIRenderer::Vertex)), 0, VriVertexStepRate_PerVertex}};

        // A quad waiting for the painter's sort.
        struct Quad
        {
            float                     Depth;
            uint32_t                  Order;
            UIRenderer::Vertex        Corners[4]; // bottom-left, bottom-right, top-right, top-left
        };

        // Corner `local` (pixels, relative to the pivot) rotated by `angle` about the pivot at `origin`.
        glm::vec2 Place(glm::vec2 origin, float angle, glm::vec2 local)
        {
            const float c = std::cos(angle), s = std::sin(angle);
            return origin + glm::vec2(c * local.x - s * local.y, s * local.x + c * local.y);
        }
    } // namespace

    UIRenderer::UIRenderer(RHI::Device& device, const ShaderCompileOptions& options) :
        m_Device(device), m_Shader(ShaderAsset::Load("Assets/Shaders/Renderer/UI.dzshader", options))
    {
        EnsureCapacity(256);
    }

    UIRenderer::~UIRenderer() { m_Device.WaitIdle(); }

    FontAtlas* UIRenderer::GetFont(const std::filesystem::path& font)
    {
        const auto& path = font.empty() ? kDefaultFont : font;
        auto&       slot = m_Fonts[path];
        if (!slot)
        {
            slot = CreateScope<FontAtlas>(m_Device, path);
            if (!slot->Valid() && path != kDefaultFont)
            {
                SNOW_LEOPARD_CORE_WARN("[UIRenderer] Font {0} unavailable, using the default font", path.generic_string());
                slot.reset();
                m_Fonts.erase(path);
                return GetFont(kDefaultFont);
            }
        }
        return slot->Valid() ? slot.get() : nullptr;
    }

    void UIRenderer::EnsureCapacity(uint32_t quads)
    {
        if (quads <= m_Capacity)
        {
            return;
        }
        m_Device.WaitIdle();
        uint32_t capacity = std::max(m_Capacity, 256u);
        while (capacity < quads)
        {
            capacity *= 2;
        }
        m_Capacity     = capacity;
        m_VertexBuffer = CreateScope<RHI::Buffer>(
            m_Device, VriBufferDesc {uint64_t(capacity) * 4 * sizeof(Vertex), 0, VriBufferUsage_VertexBuffer, VriMemoryLocation_HostUpload});
        std::vector<uint32_t> indices(size_t(capacity) * 6);
        for (uint32_t quad = 0; quad < capacity; ++quad)
        {
            const uint32_t base = quad * 4;
            const uint32_t pattern[6] {base, base + 1, base + 2, base, base + 2, base + 3};
            std::copy(std::begin(pattern), std::end(pattern), indices.begin() + size_t(quad) * 6);
        }
        m_IndexBuffer = RHI::UploadBuffer(m_Device, std::as_bytes(std::span(indices)), VriBufferUsage_IndexBuffer);
    }

    void UIRenderer::Prepare(LogicScene& scene, RHI::Extent extent, const TextureIndexer& textureIndex)
    {
        m_Extent    = extent;
        m_QuadCount = 0;
        auto& registry = scene.GetRegistry();

        std::vector<Quad> quads;
        uint32_t          order = 0;
        auto enabled = [&](entt::entity entity) {
            const auto* status = registry.try_get<EntityStatusComponent>(entity);
            return status == nullptr || status->IsEnabled;
        };

        // A rect (images, buttons): pivot at Pos, rotated about it. Corners run bottom-left first (y up); textures are
        // stored top row first, so v is flipped.
        auto addRect = [&](const UI::RectTransformComponent& rect, RHI::Texture* texture, const glm::vec4& color) {
            Quad quad {rect.Pos.z, order++, {}};
            const uint32_t index = texture != nullptr ? textureIndex(texture) : 0;
            const glm::vec2 corners[4] {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
            for (int i = 0; i < 4; ++i)
            {
                quad.Corners[i] = {Place(glm::vec2(rect.Pos), rect.RotationAngle, (corners[i] - rect.Pivot) * rect.Size),
                                   glm::vec2(corners[i].x, 1.0f - corners[i].y),
                                   PackColor(color),
                                   index};
            }
            quads.push_back(quad);
        };

        registry.view<UI::RectTransformComponent, UI::ImageComponent>().each(
            [&](entt::entity entity, UI::RectTransformComponent& rect, UI::ImageComponent& image) {
                if (enabled(entity))
                {
                    addRect(rect, image.TargetGraphic, image.Color);
                }
            });
        registry.view<UI::RectTransformComponent, UI::ButtonComponent>().each(
            [&](entt::entity entity, UI::RectTransformComponent& rect, UI::ButtonComponent& button) {
                if (!enabled(entity))
                {
                    return;
                }
                if (button.TintType == UI::ButtonTintType::Color)
                {
                    addRect(rect, button.TintColor.TargetGraphic, button.TintColor.Current);
                }
                else
                {
                    addRect(rect, button.TintTexture.TargetGraphic, glm::vec4(1));
                }
            });
        registry.view<UI::RectTransformComponent, UI::TextComponent>().each(
            [&](entt::entity entity, UI::RectTransformComponent& rect, UI::TextComponent& text) {
                if (!enabled(entity) || text.TextContent.empty())
                {
                    return;
                }
                auto* font = GetFont(text.FontFilePath);
                if (font == nullptr)
                {
                    return;
                }
                const float    scale = float(text.FontSize) / float(FontAtlas::kBaseSize);
                const auto     codepoints = FontAtlas::Decode(text.TextContent);
                const uint32_t atlas      = textureIndex(font->GetTexture()) | kSdfFlag;
                const uint32_t color      = PackColor(text.Color);

                // Line by line: measure, align about Pos.x, emit glyph quads.
                float  baseline = 0; // relative to Pos.y, lines go down
                size_t start    = 0;
                while (start <= codepoints.size())
                {
                    size_t end = codepoints.find(U'\n', start);
                    if (end == std::u32string::npos)
                    {
                        end = codepoints.size();
                    }
                    float width = 0;
                    for (size_t i = start; i < end; ++i)
                    {
                        width += font->Get(codepoints[i]).Advance + (i + 1 < end ? font->Kerning(codepoints[i], codepoints[i + 1]) : 0);
                    }
                    float pen = text.TextAlignment == UI::TextComponent::Left   ? 0 :
                                text.TextAlignment == UI::TextComponent::Center ? -width * 0.5f :
                                                                                  -width;
                    for (size_t i = start; i < end; ++i)
                    {
                        const auto& glyph = font->Get(codepoints[i]);
                        if (glyph.Visible)
                        {
                            Quad            quad {rect.Pos.z, order++, {}};
                            const glm::vec2 min = glm::vec2(pen + glyph.Bearing.x, baseline + glyph.Bearing.y - glyph.Size.y) * scale;
                            const glm::vec2 max = min + glyph.Size * scale;
                            // Atlas rows run top to bottom: the glyph's top edge is at UvMin.y.
                            const glm::vec2 position[4] {{min.x, min.y}, {max.x, min.y}, {max.x, max.y}, {min.x, max.y}};
                            const glm::vec2 uv[4] {{glyph.UvMin.x, glyph.UvMax.y},
                                                   {glyph.UvMax.x, glyph.UvMax.y},
                                                   {glyph.UvMax.x, glyph.UvMin.y},
                                                   {glyph.UvMin.x, glyph.UvMin.y}};
                            for (int k = 0; k < 4; ++k)
                            {
                                quad.Corners[k] = {Place(glm::vec2(rect.Pos), rect.RotationAngle, position[k]), uv[k], color, atlas};
                            }
                            quads.push_back(quad);
                        }
                        pen += glyph.Advance + (i + 1 < end ? font->Kerning(codepoints[i], codepoints[i + 1]) : 0);
                    }
                    baseline -= font->Ascender() * 1.25f; // line height
                    start = end + 1;
                }
            });

        for (auto& [path, font] : m_Fonts)
        {
            font->Flush();
        }

        // Painter's order: lower Pos.z first; equal depths keep images < buttons < text, then creation order.
        std::stable_sort(quads.begin(), quads.end(), [](const Quad& a, const Quad& b) { return a.Depth < b.Depth; });
        m_QuadCount = uint32_t(quads.size());
        if (m_QuadCount == 0)
        {
            return;
        }
        EnsureCapacity(m_QuadCount);
        m_Vertices.resize(size_t(m_QuadCount) * 4);
        for (size_t i = 0; i < quads.size(); ++i)
        {
            std::copy(std::begin(quads[i].Corners), std::end(quads[i].Corners), m_Vertices.begin() + i * 4);
        }
        m_VertexBuffer->Write(m_Vertices.data(), m_Vertices.size() * sizeof(Vertex));
    }

    void UIRenderer::Draw(VriCommandBuffer* cmd, VriFormat format, const std::function<void(ShaderBindings&)>& writeBindings)
    {
        if (m_QuadCount == 0)
        {
            return;
        }
        if (format != m_Format)
        {
            m_Device.WaitIdle();
            ShaderPipelineDesc desc;
            desc.ColorFormats = {format};
            desc.VertexInput  = {kAttributes, 4, kStream, 1};
            m_Bindings.reset();
            m_Pipeline = CreateScope<ShaderPipeline>(m_Device, m_Shader, m_Shader.Subshaders.at(0).Passes.at(0), desc);
            m_Bindings = CreateScope<ShaderBindings>(m_Device, *m_Pipeline);
            m_Format   = format;
        }
        writeBindings(*m_Bindings);
        m_Pipeline->Bind(cmd);
        m_Bindings->Bind(cmd);
        const glm::vec4 resolution {float(m_Extent.Width), float(m_Extent.Height), 0, 0};
        m_Pipeline->SetConstants(cmd, &resolution, sizeof(resolution));
        const VriVertexBufferBinding stream {m_VertexBuffer->Handle, 0};
        m_Device.Core.CmdSetVertexBuffers(cmd, 0, &stream, 1);
        m_Device.Core.CmdSetIndexBuffer(cmd, m_IndexBuffer->Handle, 0, VriIndexType_UInt32);
        const VriDrawIndexedDesc draw {m_QuadCount * 6, 1, 0, 0, 0};
        m_Device.Core.CmdDrawIndexed(cmd, &draw);
    }
} // namespace SnowLeopardEngine
