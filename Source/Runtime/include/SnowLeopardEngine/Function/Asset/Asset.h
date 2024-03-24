#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Object.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

namespace SnowLeopardEngine
{
    enum class AssetType : uint32_t
    {
        Invalid = 0,
        NormalFile,
        Directory,
        RawText,
        Font,
        Texture,
        AudioClip,
        Model,
        AnimationClip,
        Skeleton,
        GLSLShader,
        DzShader,
        DzMaterial,
        DzScene,
        Script
    };

    enum class AssetIconType : uint32_t
    {
        Invalid = 0,
        NormalFile,
        Directory,
        RawText,
        Font_TTF,
        Texture_PNG,
        Texture_JPG,
        Texture_TGA,
        AudioClip_MP3,
        AudioClip_WAV,
        AudioClip_OGG,
        Model_OBJ,
        Model_FBX,
        Model_GLB,
        Model_GLTF,
        Animation_OZZ,
        Skeleton_OZZ,
        DzShader,
        DzMaterial,
        DzScene,
        GLSL,
        Vert_GLSL,
        Frag_GLSL,
        Geom_GLSL,
        Comp_GLSL,
        Script_CSharp
    };

    static std::unordered_map<std::string, AssetType> ExtensionsToType = {{".mp3", AssetType::AudioClip},
                                                                          {".ogg", AssetType::AudioClip},
                                                                          {".wav", AssetType::AudioClip},
                                                                          {".jpg", AssetType::Texture},
                                                                          {".png", AssetType::Texture},
                                                                          {".tga", AssetType::Texture},
                                                                          {".cs", AssetType::Script},
                                                                          {".glsl", AssetType::GLSLShader},
                                                                          {".dzshader", AssetType::DzShader},
                                                                          {".vert", AssetType::GLSLShader},
                                                                          {".frag", AssetType::GLSLShader},
                                                                          {".txt", AssetType::RawText},
                                                                          {".dzscene", AssetType::DzScene},
                                                                          {".obj", AssetType::Model},
                                                                          {".fbx", AssetType::Model},
                                                                          {".glb", AssetType::Model},
                                                                          {".gltf", AssetType::Model},
                                                                          {".dzmaterial", AssetType::DzMaterial},
                                                                          {".ozzanim", AssetType::AnimationClip},
                                                                          {".ozzskel", AssetType::Skeleton},
                                                                          {".ttf", AssetType::Font}};

    static std::unordered_map<std::string, AssetIconType> ExtensionsToIconType = {
        {"mp3", AssetIconType::AudioClip_MP3},     {".ogg", AssetIconType::AudioClip_OGG},
        {"wav", AssetIconType::AudioClip_WAV},     {".jpg", AssetIconType::Texture_JPG},
        {"png", AssetIconType::Texture_PNG},       {".tga", AssetIconType::Texture_TGA},
        {"cs", AssetIconType::Script_CSharp},      {".glsl", AssetIconType::GLSL},
        {"dzshader", AssetIconType::DzShader},     {".dzmaterial", AssetIconType::DzMaterial},
        {"vert", AssetIconType::Vert_GLSL},        {".frag", AssetIconType::Frag_GLSL},
        {"txt", AssetIconType::RawText},           {".dzscene", AssetIconType::DzScene},
        {"obj", AssetIconType::Model_OBJ},         {".fbx", AssetIconType::Model_FBX},
        {"glb", AssetIconType::Model_GLB},         {".gltf", AssetIconType::Model_GLTF},
        {"ozzanim", AssetIconType::Animation_OZZ}, {".ozzskel", AssetIconType::Skeleton_OZZ},
        {"ttf", AssetIconType::Font_TTF}};

    class Asset : public Object
    {
    public:
        explicit Asset(const std::filesystem::path& path, const CoreUUID& uuid) : m_Path(path), m_UUID(uuid) {}

        AssetType     GetType() const { return ExtensionsToType[GetExtension()]; }
        AssetIconType GetIconType() const { return ExtensionsToIconType[GetExtension()]; }

        std::filesystem::path GetPath() const { return m_Path; }
        std::string           GetFileName() const { return m_Path.stem().string(); }
        std::string           GetExtension() const { return m_Path.extension().generic_string(); }

        CoreUUID GetUUID() const { return m_UUID; }

    protected:
        std::filesystem::path m_Path;
        CoreUUID              m_UUID;
    };
} // namespace SnowLeopardEngine