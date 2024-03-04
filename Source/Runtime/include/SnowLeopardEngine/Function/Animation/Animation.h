#pragma once

#include "SnowLeopardEngine/Function/Animation/Bone.h"

#include <assimp/scene.h>

namespace SnowLeopardEngine
{
    // forward
    class Model;

    struct AssimpNodeData
    {
        glm::mat4                   Transformation;
        std::string                 Name;
        int                         ChildrenCount;
        std::vector<AssimpNodeData> Children;
    };

    class Animation
    {
    public:
        Animation(const aiScene* scene, const aiAnimation* animation, Model* model);

        Bone* FindBone(const std::string& name);

        inline float GetTicksPerSecond() const { return m_TicksPerSecond; }

        inline float GetDuration() const { return m_Duration; }

        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

        inline const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

    private:
        void ReadMissingBones(const aiAnimation* animation, Model* model);
        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

    private:
        float                           m_Duration;
        int                             m_TicksPerSecond;
        std::vector<Bone>               m_Bones;
        AssimpNodeData                  m_RootNode;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
    };
} // namespace SnowLeopardEngine