#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace SnowLeopardEngine
{
    class ModelLoader
    {
    public:
        static bool LoadModel(const std::filesystem::path& path, Model& model);

    private:
        static void ProcessNode(const aiScene* scene, const aiNode* node, Model& model);
        static void ProcessMesh(const aiScene* scene, const aiMesh* mesh, Model& model);

        static void SetVertexBoneDataToDefault(AnimatedMeshVertexData& vertex);
        static void SetVertexBoneData(AnimatedMeshVertexData& vertex, int boneID, float weight);
        static void ExtractBoneWeightForVertices(std::vector<AnimatedMeshVertexData>& vertices, const aiMesh* mesh, Model& model);
    };
} // namespace SnowLeopardEngine