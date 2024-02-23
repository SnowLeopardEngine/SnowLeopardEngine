#include "SnowLeopardEngine/Function/Asset/Loaders/ModelLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    static Assimp::Importer g_Importer;

    bool ModelLoader::LoadModel(const std::filesystem::path& path, Model& model)
    {
        // Get aiScene
        const auto* aiScene = g_Importer.ReadFile(path.generic_string(),
                                                  aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                                                      aiProcess_GlobalScale);

        // Check
        if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode)
        {
            SNOW_LEOPARD_CORE_ERROR("[ModelLoader] Failed to load the .obj model: {0}, error message: {1}",
                                    path.filename().generic_string(),
                                    g_Importer.GetErrorString());
            return false;
        }

        // Process root node
        ProcessNode(aiScene, aiScene->mRootNode, model);

        return true;
    }

    void ModelLoader::LoadBones(const aiMesh* aiMesh, Model& model)
    {
        for (uint32_t i = 0; i < aiMesh->mNumBones; i++)
        {
            uint32_t    boneIndex = 0;
            std::string boneName(aiMesh->mBones[i]->mName.data);

            if (model.BoneMapping.find(boneName) == model.BoneMapping.end())
            {
                // Allocate an index for a new bone
                boneIndex                = model.NumBones++;
                BoneInfo    bi           = {};
                aiMatrix4x4 offsetMatrix = aiMesh->mBones[i]->mOffsetMatrix;
                bi.BoneOffset            = {offsetMatrix.a1,
                                            offsetMatrix.b1,
                                            offsetMatrix.c1,
                                            offsetMatrix.d1,
                                            offsetMatrix.a2,
                                            offsetMatrix.b2,
                                            offsetMatrix.c2,
                                            offsetMatrix.d2,
                                            offsetMatrix.a3,
                                            offsetMatrix.b3,
                                            offsetMatrix.c3,
                                            offsetMatrix.d3,
                                            offsetMatrix.a4,
                                            offsetMatrix.b4,
                                            offsetMatrix.c4,
                                            offsetMatrix.d4};

                model.BoneInfo.push_back(bi);
                model.BoneMapping[boneName] = boneIndex;
            }
            else
            {
                boneIndex = model.BoneMapping[boneName];
            }

            for (size_t j = 0; j < aiMesh->mBones[i]->mNumWeights; j++)
            {
                unsigned int vertexID = aiMesh->mBones[i]->mWeights[j].mVertexId;
                float        weight   = aiMesh->mBones[i]->mWeights[j].mWeight;
                model.Bones[vertexID].AddBoneData(boneIndex, weight);
            }
        }
    }

    void ModelLoader::ProcessNode(const aiScene* scene, const aiNode* node, Model& model)
    {
        // Process meshes
        for (uint32_t i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            ProcessMesh(scene, mesh, model);
        }

        // TODO: Process bones

        // Process children recursively
        for (uint32_t i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(scene, node->mChildren[i], model);
        }
    }

    void ModelLoader::ProcessMesh(const aiScene* scene, const aiMesh* mesh, Model& model)
    {
        MeshItem meshItem = {};
        meshItem.Name     = mesh->mName.C_Str();

        // Load vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
        {
            VertexData vertex;
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            vertex.Normal   = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

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

        // Process bones
        if (mesh->HasBones())
        {
            for (uint32_t i = 0; i < mesh->mNumBones; i++)
            {
                aiBone* bone = mesh->mBones[i];
                ProcessBone(scene, mesh, bone, model);
            }
        }
    }

    void ModelLoader::ProcessBone(const aiScene* scene, const aiMesh* mesh, const aiBone* bone, Model& model)
    {
        // TODO: Process a single bone data, save to model
    }

    void ModelLoader::ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& aarentTransform) {}
} // namespace SnowLeopardEngine