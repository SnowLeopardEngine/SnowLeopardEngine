#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"

#include "ozz/base/maths/simd_math.h"

namespace SnowLeopardEngine
{
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