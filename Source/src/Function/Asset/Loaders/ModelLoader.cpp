#include "SnowLeopardEngine/Function/Asset/Loaders/ModelLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

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
            LoadBones(aiMesh, model);
        }

        // TODO: Yanni Ma & Haodong Lin: Get Animation data

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
                boneIndex = model.NumBones;
                model.NumBones++;
                BoneInfo bi;
                model.BoneInfo.push_back(bi);
                aiMatrix4x4 offsetMatrix             = aiMesh->mBones[i]->mOffsetMatrix;
                model.BoneInfo[boneIndex].boneOffset = {offsetMatrix.a1,
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
                model.BoneMapping[boneName]          = boneIndex;
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

    void ModelLoader::ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& aarentTransform) {}
} // namespace SnowLeopardEngine