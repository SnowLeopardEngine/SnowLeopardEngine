#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Asset/OzzMesh.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include "SnowLeopardEngine/Function/Util/Util.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"

namespace SnowLeopardEngine
{
    bool OzzModelLoader::Load(const OzzModelLoadConfig& config, Model* model)
    {
        bool skinned = false;
        if (!LoadMesh(config.OzzMeshPath, model, skinned))
        {
            return false;
        }

        if (skinned)
        {
            if (!LoadAnimations(config.OzzSkeletonPath, config.OzzAnimationPaths, model))
            {
                return false;
            }
        }

        return true;
    }

    bool OzzModelLoader::LoadMesh(const std::filesystem::path& path, Model* model, bool& skinned)
    {
        ozz::io::File meshFile(path.generic_string().c_str(), "rb");
        if (!meshFile.opened())
        {
            SNOW_LEOPARD_CORE_ERROR("[OzzMeshLoader] Failed to load the ozz mesh file: {0}", path.generic_string());
            return false;
        }

        ozz::vector<ozz::sample::Mesh> meshes;
        ozz::io::IArchive              archive(&meshFile);
        while (archive.TestTag<ozz::sample::Mesh>())
        {
            meshes.resize(meshes.size() + 1);
            archive >> meshes.back();
        }

        SNOW_LEOPARD_CORE_ASSERT(meshes.size() == 1, "[OzzMeshLoader] Assertion failed, only support 1 mesh for now.");

        auto& meshInstance = meshes[0];
        skinned            = meshInstance.skinned();

        MeshItem item = {};
        item.OzzMesh  = meshInstance;

        for (const auto& part : meshInstance.parts)
        {
            for (uint32_t i = 0; i < part.vertex_count(); ++i)
            {
                MeshVertexData vertex = {};

                auto px         = part.positions[i * part.kPositionsCpnts];
                auto py         = part.positions[i * part.kPositionsCpnts + 1];
                auto pz         = part.positions[i * part.kPositionsCpnts + 2];
                vertex.Position = {px, py, pz};

                auto nx       = part.normals[i * part.kNormalsCpnts];
                auto ny       = part.normals[i * part.kNormalsCpnts + 1];
                auto nz       = part.normals[i * part.kNormalsCpnts + 2];
                vertex.Normal = {nx, ny, nz};

                auto u          = part.uvs[i * part.kUVsCpnts];
                auto v          = part.uvs[i * part.kUVsCpnts + 1];
                vertex.TexCoord = {u, 1 - v};

                item.Data.Vertices.emplace_back(vertex);
            }
        }

        item.Data.VertFormat = VertexFormat::Builder {}.BuildDefault();

        for (uint32_t indexId = 0; indexId < meshInstance.triangle_index_count(); ++indexId)
        {
            item.Data.Indices.emplace_back(meshInstance.triangle_indices[indexId]);
        }

        model->Meshes.Items.emplace_back(item);

        return true;
    }

    bool OzzModelLoader::LoadAnimations(const std::filesystem::path&              skeletonPath,
                                        const std::vector<std::filesystem::path>& animationPaths,
                                        Model*                                    model)
    {
        uint32_t animationClipCount = animationPaths.size();
        model->AnimationClips.resize(animationClipCount);

        for (uint32_t i = 0; i < animationClipCount; ++i)
        {
            model->AnimationClips[i]          = CreateRef<AnimationClip>();
            model->AnimationClips[i]->MeshPtr = &model->Meshes.Items[0];
            model->AnimationClips[i]->SkinningMatrices.resize(model->Meshes.Items[0].OzzMesh.joint_remaps.size());

            bool load = model->AnimationClips[i]->LoadFromOzzFiles(skeletonPath.generic_string(),
                                                                   animationPaths[i].generic_string());
            if (!load)
            {
                return false;
            }
        }

        return true;
    }
} // namespace SnowLeopardEngine