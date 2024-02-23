#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace SnowLeopardEngine
{
    class ModelLoader
    {
    private:
    public:
        static bool LoadModel(const std::filesystem::path& path, Model& model);
        static void LoadBones(const aiMesh* mesh, Model& model);

        void              ReadNodeHierarchy(float animationTime, const aiNode* pNode, const glm::mat4& aarentTransform);
        void              CalcInterpolatedScaling(aiVector3D& out, float animationTime, const aiNodeAnim* pNodeAnim);
        void              CalcInterpolatedRotation(aiQuaternion& out, float animationTime, const aiNodeAnim* pNodeAnim);
        void              CalcInterpolatedPosition(aiVector3D& out, float animationTime, const aiNodeAnim* pNodeAnim);
        unsigned int      FindScaling(float animationTime, const aiNodeAnim* pNodeAnim);
        unsigned int      FindRotation(float animationTime, const aiNodeAnim* pNodeAnim);
        unsigned int      FindPosition(float animationTime, const aiNodeAnim* pNodeAnim);
        const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string& nodeName);
    };
} // namespace SnowLeopardEngine