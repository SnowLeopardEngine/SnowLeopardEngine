#pragma once

#include "SnowLeopardEngine/Core/Math/AABB.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class Texture;

    enum class RenderableType
    {
        Default = 0,
        UI,
        Grass
    };

    struct RenderableUISpecs
    {
        enum class UISpecType
        {
            Image,
            Button,
            Text
        };

        UISpecType Type;

        // Button Stuff
        Texture*  ButtonColorTintTexture = nullptr;
        glm::vec4 ButtonColorTintCurrentColor;

        // Image Stuff
        Texture*  ImageTexture = nullptr;
        glm::vec4 ImageColor;

        // Text Stuff
        std::string Text;
        glm::vec4   TextColor;
        uint32_t    FontSize;
        glm::vec3   TextPos;
    };

    struct Renderable
    {
        MeshItem* Mesh        = nullptr;
        Material* Mat         = nullptr;
        glm::mat4 ModelMatrix = glm::mat4(1.0f);

        AABB BoundingBox;

        RenderableType Type;

        RenderableUISpecs UISpecs;
    };

    enum class RenderableGroupType
    {
        Default = 0,
        Instancing
    };

    struct RenderableGroup
    {
        RenderableGroupType     GroupType;
        std::vector<Renderable> Renderables;
    };

    using RenderableGroups = std::unordered_map<size_t, RenderableGroup>;

    inline bool isTransparent(const Renderable& renderable)
    {
        return renderable.Mat->GetDefine().RenderQueue == RenderQueue_Transparent;
    }

    inline bool isOpaque(const Renderable& renderable) { return !isTransparent(renderable); }

    inline bool isDefault(const Renderable& renderable) { return renderable.Type == RenderableType::Default; }
    inline bool isUI(const Renderable& renderable) { return renderable.Type == RenderableType::UI; }
    inline bool isGrass(const Renderable& renderable) { return renderable.Type == RenderableType::Grass; }
} // namespace SnowLeopardEngine