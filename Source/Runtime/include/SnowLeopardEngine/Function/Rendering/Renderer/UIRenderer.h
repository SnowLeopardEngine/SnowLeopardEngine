#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/Renderer/FontAtlas.h"
#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderAsset.h"
#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderPipeline.h"

#include <functional>
#include <map>
#include <vector>

namespace SnowLeopardEngine
{
    class LogicScene;

    // In-game UI (images, buttons, text) as one batch: every quad of the frame goes into one vertex buffer, sorted
    // back to front by RectTransform Pos.z, and is drawn with a single indexed draw. Textures come from the scene
    // renderer's bindless table; text uses SDF font atlases. Drawn after tone mapping, so UI colours are exact.
    //
    // Coordinates are window pixels from the bottom-left corner. A RectTransform places its rect's pivot at Pos and
    // rotates the rect about it (RotationAngle, radians). Text: FontSize is the em size in pixels; Pos is the start
    // of the first baseline (Left), its middle (Center) or its end (Right); '\n' starts a new line.
    class UIRenderer
    {
    public:
        static inline const std::filesystem::path kDefaultFont = "Assets/Fonts/Jaro-Regular.ttf";

        using TextureIndexer = std::function<uint32_t(RHI::Texture*)>;

        UIRenderer(RHI::Device& device, const ShaderCompileOptions& options);
        ~UIRenderer();

        UIRenderer(const UIRenderer&)            = delete;
        UIRenderer& operator=(const UIRenderer&) = delete;

        // Builds this frame's quads (and uploads glyphs added to an atlas). Before the frame is recorded.
        void     Prepare(LogicScene& scene, RHI::Extent extent, const TextureIndexer& textureIndex);
        uint32_t GetQuadCount() const { return m_QuadCount; }

        // Inside a render pass on a `format` target; `writeBindings` fills the renderer resources of set 0.
        void Draw(VriCommandBuffer* cmd, VriFormat format, const std::function<void(ShaderBindings&)>& writeBindings);

        FontAtlas* GetFont(const std::filesystem::path& font);
        void       Clear() { m_Fonts.clear(); } // fonts refer to the bindless table of the previous scene

        struct Vertex
        {
            glm::vec2 Position; // pixels, bottom-left origin
            glm::vec2 Uv;
            uint32_t  Color;    // RGBA8, straight alpha
            uint32_t  Texture;  // bindless index (0: none); top bit: SDF glyph
        };

    private:
        void EnsureCapacity(uint32_t quads);

        RHI::Device&                                      m_Device;
        ShaderAsset                                       m_Shader;
        Scope<ShaderPipeline>                             m_Pipeline;
        Scope<ShaderBindings>                             m_Bindings;
        VriFormat                                         m_Format = VriFormat_Unknown;
        std::map<std::filesystem::path, Scope<FontAtlas>> m_Fonts;
        std::vector<Vertex>                               m_Vertices;
        Scope<RHI::Buffer>                                m_VertexBuffer, m_IndexBuffer;
        uint32_t                                          m_Capacity  = 0;
        uint32_t                                          m_QuadCount = 0;
        RHI::Extent                                       m_Extent {};
    };
} // namespace SnowLeopardEngine
