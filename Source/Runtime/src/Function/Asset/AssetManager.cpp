#include "SnowLeopardEngine/Function/Asset/AssetManager.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <SAssetPipeline/Assets.h>

#include <cstring>
#include <mutex>
#include <unordered_set>

namespace SnowLeopardEngine
{
    namespace
    {
        namespace AP = AssetPipeline;

        struct State
        {
            Scope<AP::AssetRegistry>                               Registry;
            std::unordered_map<CoreUUID, Scope<RHI::Texture>>      Textures;
            std::unordered_map<CoreUUID, Ref<MeshGroup>>           Meshes;
            std::unordered_map<CoreUUID, Scope<Material>>          Materials;
            std::unordered_set<std::string>                        WarnedFallbacks;
            AssetManager::Statistics                               Stats;
        };

        State& GetState()
        {
            static State state;
            return state;
        }

        AP::AssetId ToAssetId(const CoreUUID& id)
        {
            AP::AssetId result;
            const auto  bytes = id.as_bytes();
            std::memcpy(result.Bytes.data(), bytes.data(), 16);
            return result;
        }

        CoreUUID ToCoreUUID(const AP::AssetId& id)
        {
            std::array<uuids::uuid::value_type, 16> bytes {};
            std::memcpy(bytes.data(), id.Bytes.data(), 16);
            return CoreUUID(bytes);
        }

        const AP::RegistryEntry* Entry(const CoreUUID& id, AP::AssetType type)
        {
            auto& state = GetState();
            if (!state.Registry)
                return nullptr;
            const auto* entry = state.Registry->Find(ToAssetId(id));
            return entry != nullptr && entry->Type == type ? entry : nullptr;
        }

        // The engine samples colour textures as UNORM and decodes sRGB in the shader (VRI has no sRGB BC formats), so the
        // sRGB flag only matters to the importer's mip filtering.
        VriFormat ToVriFormat(AP::TextureFormat format)
        {
            switch (format)
            {
                case AP::TextureFormat::BC7:
                case AP::TextureFormat::BC7Srgb:
                    return VriFormat_BC7_UNORM;
                case AP::TextureFormat::RGBA16F:
                    return VriFormat_RGBA16_SFLOAT;
                default:
                    return VriFormat_RGBA8_UNORM;
            }
        }
    } // namespace

    bool AssetManager::Mount(const std::filesystem::path& root)
    {
        auto& state    = GetState();
        state.Registry = CreateScope<AP::AssetRegistry>(std::filesystem::absolute(root).lexically_normal());
        std::string error;
        if (!state.Registry->Load(&error))
        {
            SNOW_LEOPARD_CORE_ERROR("[AssetManager] {0}", error);
            state.Registry.reset();
            return false;
        }
        if (state.Registry->GetEntries().empty())
        {
            SNOW_LEOPARD_CORE_WARN("[AssetManager] {0} has no imported assets (run `sap import`): source files are parsed at runtime",
                                   root.generic_string());
            return false;
        }
        SNOW_LEOPARD_CORE_INFO("[AssetManager] Mounted {0}: {1} imported assets", root.generic_string(), state.Registry->GetEntries().size());
        return true;
    }

    void AssetManager::Unmount()
    {
        auto& state = GetState();
        state.Materials.clear();
        state.Meshes.clear();
        state.Textures.clear();
        state.Registry.reset();
        state.WarnedFallbacks.clear();
        state.Stats = {};
    }

    bool AssetManager::IsMounted() { return GetState().Registry != nullptr; }

    const AssetPipeline::AssetRegistry* AssetManager::GetRegistry() { return GetState().Registry.get(); }

    const AssetManager::Statistics& AssetManager::GetStatistics() { return GetState().Stats; }

    void AssetManager::ReportFallback(const std::filesystem::path& source)
    {
        auto& state = GetState();
        ++state.Stats.FallbackLoads;
        if (state.WarnedFallbacks.insert(source.generic_string()).second)
        {
            SNOW_LEOPARD_CORE_WARN("[AssetManager] {0} is not imported: parsed at runtime", source.generic_string());
        }
    }

    std::optional<CoreUUID> AssetManager::FindBySource(const std::filesystem::path& source)
    {
        auto& state = GetState();
        if (!state.Registry)
            return std::nullopt;
        const auto* entry = state.Registry->FindBySource(source.generic_string());
        return entry != nullptr ? std::optional<CoreUUID>(ToCoreUUID(entry->Id)) : std::nullopt;
    }

    RHI::Texture* AssetManager::LoadTexture(const CoreUUID& id)
    {
        auto& state = GetState();
        if (auto found = state.Textures.find(id); found != state.Textures.end())
            return found->second.get();
        const auto* entry = Entry(id, AP::AssetType::Texture);
        if (entry == nullptr)
            return nullptr;
        std::string error;
        const auto  asset = AP::LoadTexture(state.Registry->GetCookedPath(*entry), &error);
        if (!asset)
        {
            SNOW_LEOPARD_CORE_ERROR("[AssetManager] {0}: {1}", entry->Source, error);
            return nullptr;
        }

        auto& device = g_EngineContext->RenderSys->GetDevice();
        auto  desc   = RHI::ColorTextureDesc({asset->Width, asset->Height}, ToVriFormat(asset->Format));
        desc.usage   = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
        desc.mipNum  = uint32_t(asset->Mips.size());
        auto texture = CreateScope<RHI::Texture>(device, desc);
        std::vector<std::span<const std::byte>> levels;
        for (const auto& mip : asset->Mips)
            levels.emplace_back(reinterpret_cast<const std::byte*>(mip.data()), mip.size());
        RHI::UploadTexture(device, *texture, levels);

        ++state.Stats.CookedLoads;
        auto* result      = texture.get();
        state.Textures[id] = std::move(texture);
        return result;
    }

    Ref<MeshGroup> AssetManager::LoadMesh(const CoreUUID& id)
    {
        auto& state = GetState();
        if (auto found = state.Meshes.find(id); found != state.Meshes.end())
            return found->second;
        const auto* entry = Entry(id, AP::AssetType::Mesh);
        if (entry == nullptr)
            return nullptr;
        std::string error;
        const auto  asset = AP::LoadMesh(state.Registry->GetCookedPath(*entry), &error);
        if (!asset)
        {
            SNOW_LEOPARD_CORE_ERROR("[AssetManager] {0}: {1}", entry->Source, error);
            return nullptr;
        }

        // One MeshItem per sub-mesh (the renderer's unit); vertices are already MeshVertexData.
        static_assert(sizeof(MeshVertexData) == sizeof(AP::MeshVertex), "cooked vertices must match MeshVertexData");
        auto group = CreateRef<MeshGroup>();
        for (const auto& sub : asset->SubMeshes)
        {
            auto& item = group->Items.emplace_back();
            item.Name  = sub.Name;
            item.Data.Vertices.resize(sub.VertexCount);
            std::memcpy(item.Data.Vertices.data(), asset->Vertices.data() + sub.FirstVertex, sub.VertexCount * sizeof(MeshVertexData));
            item.Data.Indices.assign(asset->Indices.begin() + sub.FirstIndex, asset->Indices.begin() + sub.FirstIndex + sub.IndexCount);
        }
        ++state.Stats.CookedLoads;
        state.Meshes[id] = group;
        return group;
    }

    Material* AssetManager::LoadMaterial(const CoreUUID& id)
    {
        auto& state = GetState();
        if (auto found = state.Materials.find(id); found != state.Materials.end())
            return found->second.get();
        const auto* entry = Entry(id, AP::AssetType::Material);
        if (entry == nullptr)
            return nullptr;
        std::string error;
        const auto  asset = AP::LoadMaterial(state.Registry->GetCookedPath(*entry), &error);
        if (!asset)
        {
            SNOW_LEOPARD_CORE_ERROR("[AssetManager] {0}: {1}", entry->Source, error);
            return nullptr;
        }

        DzMaterial define;
        define.Name             = asset->Name;
        define.RenderQueue      = asset->RenderQueue;
        define.TextureFlipY     = asset->TextureFlipY;
        define.TileSize         = asset->TileSize;
        define.EnableInstancing = asset->EnableInstancing;
        for (const auto& property : asset->Properties)
        {
            DzMaterialProperty value {property.Name, property.Type, property.Value};
            if (!property.Texture.IsNil())
                value.TextureId = ToCoreUUID(property.Texture);
            define.Properties.push_back(std::move(value));
        }
        ++state.Stats.CookedLoads;
        auto material        = CreateScope<Material>(define);
        auto* result         = material.get();
        state.Materials[id]  = std::move(material);
        return result;
    }
} // namespace SnowLeopardEngine
