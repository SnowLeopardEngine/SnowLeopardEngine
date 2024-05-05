#include "SnowLeopardEngine/Function/IO/AssimpModelLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    static Assimp::Importer g_Importer;

    bool AssimpModelLoader::LoadModel(const std::filesystem::path& path, Model& model)
    {
        model.SourcePath = path.generic_string();

        // Get aiScene
        const auto* scene = g_Importer.ReadFile(path.generic_string(),
                                                aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                                                    aiProcess_GlobalScale);

        // Check
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            SNOW_LEOPARD_CORE_ERROR("[AssimpModelLoader] Failed to load the .obj model: {0}, error message: {1}",
                                    path.filename().generic_string(),
                                    g_Importer.GetErrorString());
            return false;
        }

        // Process root node
        ProcessNode(scene, scene->mRootNode, model);

        return true;
    }

    void AssimpModelLoader::ProcessNode(const aiScene* scene, const aiNode* node, Model& model)
    {
        // Process meshes
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(scene, mesh, model);
        }

        // Process children recursively
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(scene, node->mChildren[i], model);
        }
    }

    void AssimpModelLoader::ProcessMesh(const aiScene* scene, const aiMesh* mesh, Model& model)
    {
        MeshItem meshItem        = {};
        meshItem.Name            = mesh->mName.C_Str();
        meshItem.Data.VertFormat = VertexFormat::Builder {}.BuildDefault();

        bool hasBones = mesh->HasBones();

        // Load vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
        {
            MeshVertexData vertex = {};
            vertex.Position       = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal         = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            if (mesh->HasTextureCoords(0))
            {
                vertex.TexCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }

            meshItem.Data.Vertices.push_back(vertex);
        }

        // Load indices
        for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j)
            {
                meshItem.Data.Indices.push_back(face.mIndices[j]);
            }
        }

        model.Meshes.Items.emplace_back(meshItem);
    }
} // namespace SnowLeopardEngine