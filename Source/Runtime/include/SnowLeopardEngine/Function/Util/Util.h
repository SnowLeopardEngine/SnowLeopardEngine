#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"

#include "ozz/base/maths/simd_math.h"
#include <PxPhysicsAPI.h>
#include <assimp/scene.h>

namespace SnowLeopardEngine
{
    class TransformComponent;

    class AssimpGLMHelpers
    {
    public:
        static inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
        {
            // clang-format off
            glm::mat4 to;
            //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
            to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
            to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
            to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
            to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
            // clang-format on
            return to;
        }

        static inline glm::vec3 GetGLMVec(const aiVector3D& vec) { return glm::vec3(vec.x, vec.y, vec.z); }

        static inline glm::quat GetGLMQuat(const aiQuaternion& pOrientation)
        {
            return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
        }
    };

    class PhysXGLMHelpers
    {
    public:
        static inline physx::PxVec3 GetPhysXVec3(const glm::vec3& vec) { return physx::PxVec3(vec.x, vec.y, vec.z); }
        static inline physx::PxQuat GetPhysXQuat(const glm::quat& q) { return physx::PxQuat(q.x, q.y, q.z, q.w); }
        static physx::PxTransform   GetPhysXTransform(TransformComponent* transform);

        static inline glm::vec3 GetGLMVec3(const physx::PxVec3& vec) { return glm::vec3(vec.x, vec.y, vec.z); }
    };

    class OzzGLMHelpers
    {
    public:
        static ozz::math::Float4x4 GetOzzFloat4x4(const glm::mat4& mat)
        {
            ozz::math::Float4x4 result;
            for (int i = 0; i < 4; ++i)
            {
                result.cols[i] = ozz::math::simd_float4::Load(mat[0][i], mat[1][i], mat[2][i], mat[3][i]);
            }
            return result;
        }

        static glm::mat4 GetGLMMat4(const ozz::math::Float4x4& ozzMat)
        {
            glm::mat4 result;
            for (int i = 0; i < 4; ++i)
            {
                result[0][i] = ozz::math::GetX(ozzMat.cols[i]);
                result[1][i] = ozz::math::GetY(ozzMat.cols[i]);
                result[2][i] = ozz::math::GetZ(ozzMat.cols[i]);
                result[3][i] = ozz::math::GetW(ozzMat.cols[i]);
            }
            return result;
        }
    };
} // namespace SnowLeopardEngine