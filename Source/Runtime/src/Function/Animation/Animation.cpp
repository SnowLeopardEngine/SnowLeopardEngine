#include "SnowLeopardEngine/Function/Animation/Animation.h"
#include "SnowLeopardEngine/Function/Asset/Model.h"
#include "SnowLeopardEngine/Function/Util/Util.h"

namespace SnowLeopardEngine
{
    Animation::Animation(const aiScene* scene, const aiAnimation* animation, Model* model)
    {
        m_Duration       = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        ReadHeirarchyData(m_RootNode, scene->mRootNode);
        ReadMissingBones(animation, model);
    }

    Bone* Animation::FindBone(const std::string& name)
    {
        auto iter =
            std::find_if(m_Bones.begin(), m_Bones.end(), [&](const Bone& bone) { return bone.GetBoneName() == name; });
        if (iter == m_Bones.end())
            return nullptr;
        else
            return &(*iter);
    }

    void Animation::ReadMissingBones(const aiAnimation* animation, Model* model)
    {
        int size = animation->mNumChannels;

        auto& boneInfoMap = model->BoneInfoMap; // getting m_BoneInfoMap from Model class
        int&  boneCount   = model->BoneCounter; // getting the m_BoneCounter from Model class

        // reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto*       channel  = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].Id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].Id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        dest.Name           = src->mName.data;
        dest.Transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.ChildrenCount  = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.Children.push_back(newData);
        }
    }
} // namespace SnowLeopardEngine