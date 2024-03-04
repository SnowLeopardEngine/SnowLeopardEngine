#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Util/Util.h"

#include <assimp/scene.h>

namespace SnowLeopardEngine
{
    struct KeyPosition
    {
        glm::vec3 Position;
        float     TimeStamp;
    };

    struct KeyRotation
    {
        glm::quat Orientation;
        float     TimeStamp;
    };

    struct KeyScale
    {
        glm::vec3 Scale;
        float     TimeStamp;
    };

    struct BoneInfo
    {
        // id is index in finalBoneMatrices
        int Id = -1;

        // offset matrix transforms vertex from model space to bone space
        glm::mat4 Offset;
    };

    class Bone
    {
    public:
        /*reads keyframes from aiNodeAnim*/
        Bone(const std::string& name, int id, const aiNodeAnim* channel) :
            m_Name(name), m_ID(id), m_LocalTransform(1.0f)
        {
            m_NumPositions = channel->mNumPositionKeys;

            for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
            {
                auto        aiPosition = channel->mPositionKeys[positionIndex].mValue;
                float       timeStamp  = channel->mPositionKeys[positionIndex].mTime;
                KeyPosition data;
                data.Position  = AssimpGLMHelpers::GetGLMVec(aiPosition);
                data.TimeStamp = timeStamp;
                m_Positions.push_back(data);
            }

            m_NumRotations = channel->mNumRotationKeys;
            for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
            {
                auto        aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
                float       timeStamp     = channel->mRotationKeys[rotationIndex].mTime;
                KeyRotation data;
                data.Orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
                data.TimeStamp   = timeStamp;
                m_Rotations.push_back(data);
            }

            m_NumScalings = channel->mNumScalingKeys;
            for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
            {
                auto     scale     = channel->mScalingKeys[keyIndex].mValue;
                float    timeStamp = channel->mScalingKeys[keyIndex].mTime;
                KeyScale data;
                data.Scale     = AssimpGLMHelpers::GetGLMVec(scale);
                data.TimeStamp = timeStamp;
                m_Scales.push_back(data);
            }
        }

        /*interpolates  b/w positions,rotations & scaling keys based on the curren time of
        the animation and prepares the local transformation matrix by combining all keys
        tranformations*/
        void Update(float animationTime)
        {
            glm::mat4 translation = InterpolatePosition(animationTime);
            glm::mat4 rotation    = InterpolateRotation(animationTime);
            glm::mat4 scale       = InterpolateScaling(animationTime);
            m_LocalTransform      = translation * rotation * scale;
        }

        glm::mat4   GetLocalTransform() const { return m_LocalTransform; }
        std::string GetBoneName() const { return m_Name; }
        int         GetBoneID() const { return m_ID; }

        /* Gets the current index on mKeyPositions to interpolate to based on
        the current animation time*/
        int GetPositionIndex(float animationTime) const
        {
            for (int index = 0; index < m_NumPositions - 1; ++index)
            {
                if (animationTime < m_Positions[index + 1].TimeStamp)
                    return index;
            }
            return -1;
        }

        /* Gets the current index on mKeyRotations to interpolate to based on the
        current animation time*/
        int GetRotationIndex(float animationTime) const
        {
            for (int index = 0; index < m_NumRotations - 1; ++index)
            {
                if (animationTime < m_Rotations[index + 1].TimeStamp)
                    return index;
            }
            return -1;
        }

        /* Gets the current index on mKeyScalings to interpolate to based on the
        current animation time */
        int GetScaleIndex(float animationTime) const
        {
            for (int index = 0; index < m_NumScalings - 1; ++index)
            {
                if (animationTime < m_Scales[index + 1].TimeStamp)
                    return index;
            }
            return -1;
        }

    private:
        /* Gets normalized value for Lerp & Slerp*/
        static float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
        {
            float scaleFactor  = 0.0f;
            float midWayLength = animationTime - lastTimeStamp;
            float framesDiff   = nextTimeStamp - lastTimeStamp;
            scaleFactor        = midWayLength / framesDiff;
            return scaleFactor;
        }

        /*figures out which position keys to interpolate b/w and performs the interpolation
        and returns the translation matrix*/
        glm::mat4 InterpolatePosition(float animationTime) const
        {
            if (1 == m_NumPositions)
                return glm::translate(glm::mat4(1.0f), m_Positions[0].Position);

            int   p0Index = GetPositionIndex(animationTime);
            int   p1Index = p0Index + 1;
            float scaleFactor =
                GetScaleFactor(m_Positions[p0Index].TimeStamp, m_Positions[p1Index].TimeStamp, animationTime);
            glm::vec3 finalPosition =
                glm::mix(m_Positions[p0Index].Position, m_Positions[p1Index].Position, scaleFactor);
            return glm::translate(glm::mat4(1.0f), finalPosition);
        }

        /*figures out which rotations keys to interpolate b/w and performs the interpolation
        and returns the rotation matrix*/
        glm::mat4 InterpolateRotation(float animationTime) const
        {
            if (1 == m_NumRotations)
            {
                auto rotation = glm::normalize(m_Rotations[0].Orientation);
                return glm::mat4_cast(rotation);
            }

            int   p0Index = GetRotationIndex(animationTime);
            int   p1Index = p0Index + 1;
            float scaleFactor =
                GetScaleFactor(m_Rotations[p0Index].TimeStamp, m_Rotations[p1Index].TimeStamp, animationTime);
            glm::quat finalRotation =
                glm::slerp(m_Rotations[p0Index].Orientation, m_Rotations[p1Index].Orientation, scaleFactor);
            finalRotation = glm::normalize(finalRotation);
            return glm::mat4_cast(finalRotation);
        }

        /*figures out which scaling keys to interpolate b/w and performs the interpolation
        and returns the scale matrix*/
        glm::mat4 InterpolateScaling(float animationTime)
        {
            if (1 == m_NumScalings)
                return glm::scale(glm::mat4(1.0f), m_Scales[0].Scale);

            int   p0Index     = GetScaleIndex(animationTime);
            int   p1Index     = p0Index + 1;
            float scaleFactor = GetScaleFactor(m_Scales[p0Index].TimeStamp, m_Scales[p1Index].TimeStamp, animationTime);
            glm::vec3 finalScale = glm::mix(m_Scales[p0Index].Scale, m_Scales[p1Index].Scale, scaleFactor);
            return glm::scale(glm::mat4(1.0f), finalScale);
        }

    private:
        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale>    m_Scales;
        int                      m_NumPositions;
        int                      m_NumRotations;
        int                      m_NumScalings;

        glm::mat4   m_LocalTransform;
        std::string m_Name;
        int         m_ID;
    };
} // namespace SnowLeopardEngine