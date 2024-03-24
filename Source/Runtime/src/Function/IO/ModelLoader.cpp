#include "SnowLeopardEngine/Function/IO/ModelLoader.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Animation.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Util/Util.h"

namespace SnowLeopardEngine
{
    static Assimp::Importer g_Importer;

    bool ModelLoader::LoadModel(const std::filesystem::path& path, Model& model)
    {
        model.SourcePath = path.generic_string();

        // Get aiScene
        const auto* scene = g_Importer.ReadFile(path.generic_string(),
                                                aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals |
                                                    aiProcess_GlobalScale);

        // Check
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            SNOW_LEOPARD_CORE_ERROR("[ModelLoader] Failed to load the .obj model: {0}, error message: {1}",
                                    path.filename().generic_string(),
                                    g_Importer.GetErrorString());
            return false;
        }

        // Process root node
        ProcessNode(scene, scene->mRootNode, model);

        // Process animations
        ProcessAnimation(scene, model);

        return true;
    }

    void ModelLoader::ProcessAnimation(const aiScene* scene, Model& model)
    {
        // Process animations
        if (scene->HasAnimations())
        {
            for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
            {
                auto* aiAnimation = scene->mAnimations[i];

                auto animation = CreateRef<Animation>(scene, aiAnimation, &model);
                model.Animations.emplace_back(animation);
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

        bool hasBones = mesh->HasBones();

        // Load vertices
        for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
        {
            AnimatedMeshVertexData vertex = {};
            vertex.Position               = AssimpGLMHelpers::GetGLMVec(mesh->mVertices[i]);
            vertex.Normal                 = AssimpGLMHelpers::GetGLMVec(mesh->mNormals[i]);

            if (mesh->HasTextureCoords(0))
            {
                vertex.TexCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
            }

            // Set default bone data
            if (hasBones)
            {
                SetVertexBoneDataToDefault(vertex);
            }

            if (hasBones)
            {
                meshItem.Data.AnimatedVertices.push_back({vertex});
            }
            else
            {
                meshItem.Data.StaticVertices.push_back(
                    StaticMeshVertexData {vertex.Position, vertex.Normal, vertex.TexCoord, vertex.EntityID});
            }
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

        // Load textures
        auto* material = scene->mMaterials[mesh->mMaterialIndex];

        // Process bones
        if (hasBones)
        {
            ExtractBoneWeightForVertices(meshItem.Data.AnimatedVertices, mesh, model);
        }

        model.Meshes.Items.emplace_back(meshItem);
    }

    void ModelLoader::SetVertexBoneDataToDefault(AnimatedMeshVertexData& vertex)
    {
        // Setup default bone data
        for (uint32_t i = 0; i < MaxBoneInfluence; ++i)
        {
            PerVertexAnimationAttributes attributes = {};
            attributes.BoneIDs[i]                   = -1;
            attributes.Weights[i]                   = 0;
            vertex.AnimationAttributes              = attributes;
        }
    }

    void ModelLoader::SetVertexBoneData(AnimatedMeshVertexData& vertex, int boneID, float weight)
    {
        for (uint32_t i = 0; i < MaxBoneInfluence; ++i)
        {
            if (vertex.AnimationAttributes.BoneIDs[i] < 0)
            {
                vertex.AnimationAttributes.Weights[i] = weight;
                vertex.AnimationAttributes.BoneIDs[i] = boneID;
                break;
            }
        }
    }

    void ModelLoader::ExtractBoneWeightForVertices(std::vector<AnimatedMeshVertexData>& vertices,
                                                   const aiMesh*                        mesh,
                                                   Model&                               model)
    {
        for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int         boneID   = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
            if (model.BoneInfoMap.find(boneName) == model.BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.Id              = model.BoneCounter;
                auto offsetMatrix           = mesh->mBones[boneIndex]->mOffsetMatrix;
                newBoneInfo.Offset          = AssimpGLMHelpers::ConvertMatrixToGLMFormat(offsetMatrix);
                model.BoneInfoMap[boneName] = newBoneInfo;
                boneID                      = model.BoneCounter;
                model.BoneCounter++;
            }
            else
            {
                boneID = model.BoneInfoMap[boneName].Id;
            }
            SNOW_LEOPARD_CORE_ASSERT(boneID != -1, "[ModelLoader] Bone ID is -1!");
            auto* weights    = mesh->mBones[boneIndex]->mWeights;
            int   numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (uint32_t weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int   vertexId = weights[weightIndex].mVertexId;
                float weight   = weights[weightIndex].mWeight;
                SNOW_LEOPARD_CORE_ASSERT(vertexId <= vertices.size(), "[ModelLoader] Vertex ID out of range!");
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }
} // namespace SnowLeopardEngine