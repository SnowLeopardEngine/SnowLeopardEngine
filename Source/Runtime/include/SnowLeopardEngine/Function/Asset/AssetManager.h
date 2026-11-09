#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

#include <SAssetPipeline/AssetRegistry.h>

#include <filesystem>
#include <optional>

namespace SnowLeopardEngine
{
    namespace RHI
    {
        class Texture;
    }
    class Material;
    struct MeshGroup;

    // Cooked assets (SAssetPipeline): the registry of an asset root, imported by `sap import` (a build step) into
    // `<root>/.imported/`. Loading is a read and an upload: textures come with their mips (BC7 where possible), meshes
    // in the renderer's vertex layout, materials with their texture ids.
    //
    // The IO loaders (textures, materials) and mesh loading in LogicScene try here first and fall back to parsing the
    // source file when an asset is not imported; fallbacks are counted (and logged once) so they can be found and the
    // fallback removed once everything is migrated.
    class AssetManager
    {
    public:
        // Reads `<root>/.imported/registry.json`. False: no registry (nothing imported), every load falls back.
        static bool Mount(const std::filesystem::path& root);
        // Releases every loaded asset (GPU textures included): call while the device is alive.
        static void Unmount();
        static bool IsMounted();

        // The asset imported from `source` (relative to the root, or "Assets/..." as scenes and materials write it).
        static std::optional<CoreUUID> FindBySource(const std::filesystem::path& source);

        // Cached; null if the id is unknown or the cooked file cannot be read.
        static RHI::Texture*  LoadTexture(const CoreUUID& id);
        static Ref<MeshGroup> LoadMesh(const CoreUUID& id);
        static Material*      LoadMaterial(const CoreUUID& id);

        struct Statistics
        {
            uint32_t CookedLoads   = 0; // assets read from cooked files
            uint32_t FallbackLoads = 0; // source files parsed at runtime (not imported)
        };
        static const Statistics& GetStatistics();
        // Called by the raw loaders: counts the fallback and warns once per path.
        static void ReportFallback(const std::filesystem::path& source);

        static const AssetPipeline::AssetRegistry* GetRegistry();
    };
} // namespace SnowLeopardEngine
