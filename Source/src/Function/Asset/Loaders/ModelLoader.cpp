#include "SnowLeopardEngine/Function/Asset/Loaders/ModelLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace SnowLeopardEngine
{
    static Assimp::Importer g_Importer;

    bool ModelLoader::LoadModel(const std::filesystem::path& path, Model& model)
    {
        // Get aiScene
        const auto* aiScene = g_Importer.ReadFile(path.generic_string(),
                                                  aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        // Check
        if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode)
        {
            SNOW_LEOPARD_CORE_ERROR("[ModelLoader] Failed to load the .obj model: {0}, error message: {1}",
                                    path.filename().generic_string(),
                                    g_Importer.GetErrorString());
            return false;
        }

        // Get mesh data
        for (unsigned int i = 0; i < aiScene->mNumMeshes; ++i)
        {
            auto* aiMesh = aiScene->mMeshes[i];

            MeshItem meshItem = {};
            meshItem.Name     = aiMesh->mName.C_Str();

            // Load vertices
            for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
            {
                VertexData vertex;
                vertex.Position = glm::vec3(aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z);
                vertex.Normal   = glm::vec3(aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z);

                if (aiMesh->HasTextureCoords(0))
                {
                    vertex.TexCoord = glm::vec2(aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y);
                }

                meshItem.Data.Vertices.push_back(vertex);
            }

            // Load indices
            for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
            {
                const aiFace& face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; ++j)
                {
                    meshItem.Data.Indices.push_back(face.mIndices[j]);
                }
            }

            model.Meshes.Items.emplace_back(meshItem);
        }

        // TODO: Yanni Ma & Haodong Lin: Get Animation data

        return true;
    }
} // namespace SnowLeopardEngine