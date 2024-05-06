#pragma once

#include "SnowLeopardEngine/Function/Asset/Model.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace SnowLeopardEngine
{
    class AssimpModelLoader
    {
    public:
        static bool LoadModel(const std::filesystem::path& path, Model& model);

    private:
        static void ProcessNode(const aiScene* scene, const aiNode* node, Model& model);
        static void ProcessMesh(const aiScene* scene, const aiMesh* mesh, Model& model);
    };
} // namespace SnowLeopardEngine