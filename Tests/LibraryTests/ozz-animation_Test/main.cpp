// ozz-animation headers
#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/base/containers/vector.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/soa_transform.h"
#include "ozz/base/maths/vec_float.h"

// raylib
#include "raylib.h"

#include <iostream>
#include <string>
#include <vector>

using namespace ozz::math;

struct OzzRuntimeInfo
{
    ozz::animation::Skeleton             Skeleton;
    ozz::animation::Animation            Animation;
    ozz::animation::SamplingJob::Context SamplingJobContext;
    ozz::vector<ozz::math::SoaTransform> LocalTransforms;
    ozz::vector<ozz::math::Float4x4>     ModelMatrices;
};

OzzRuntimeInfo* g_Ozz;

bool LoadSkeletonData(const std::string& fileName)
{
    ozz::io::File skeletonFile(fileName.c_str(), "rb");
    if (!skeletonFile.opened())
    {
        return false;
    }

    ozz::io::IArchive archive(&skeletonFile);

    if (!archive.TestTag<ozz::animation::Skeleton>())
    {
        return false;
    }

    archive >> g_Ozz->Skeleton;
    const int numSoaJoints = g_Ozz->Skeleton.num_soa_joints();
    const int numJoints    = g_Ozz->Skeleton.num_joints();
    g_Ozz->LocalTransforms.resize(numSoaJoints);
    g_Ozz->ModelMatrices.resize(numJoints);
    g_Ozz->SamplingJobContext.Resize(numJoints);

    return true;
}

bool LoadAnimationData(const std::string& fileName)
{
    ozz::io::File animationFile(fileName.c_str(), "rb");
    if (!animationFile.opened())
    {
        return false;
    }

    ozz::io::IArchive archive(&animationFile);

    if (!archive.TestTag<ozz::animation::Animation>())
    {
        return false;
    }

    archive >> g_Ozz->Animation;

    return true;
}

void MyDrawLine3D(const SimdFloat4& a, const SimdFloat4& b)
{
    Vector3 va = {GetX(a), GetY(a), GetZ(a)};
    Vector3 vb = {GetX(b), GetY(b), GetZ(b)};

    DrawLine3D(va, vb, RAYWHITE);
}

void SimpleDrawJoint(int currentJointIndex, int parentJointIndex)
{
    if (parentJointIndex < 0)
    {
        return;
    }

    const Float4x4& m0 = g_Ozz->ModelMatrices[currentJointIndex];
    const Float4x4& m1 = g_Ozz->ModelMatrices[parentJointIndex];

    const SimdFloat4 p0 = m0.cols[3];
    const SimdFloat4 p1 = m1.cols[3];

    MyDrawLine3D(p0, p1);
}

void DrawJoint(int currentJointIndex, int parentJointIndex)
{
    if (parentJointIndex < 0)
    {
        return;
    }

    const Float4x4& m0 = g_Ozz->ModelMatrices[currentJointIndex];
    const Float4x4& m1 = g_Ozz->ModelMatrices[parentJointIndex];

    const SimdFloat4 p0 = m0.cols[3];
    const SimdFloat4 p1 = m1.cols[3];
    const SimdFloat4 ny = m1.cols[1];
    const SimdFloat4 nz = m1.cols[2];

    const SimdFloat4 len = SplatX(Length3(p1 - p0)) * simd_float4::Load1(0.1f);

    const SimdFloat4 pmid = p0 + (p1 - p0) * simd_float4::Load1(0.66f);
    const SimdFloat4 p2   = pmid + ny * len;
    const SimdFloat4 p3   = pmid + nz * len;
    const SimdFloat4 p4   = pmid - ny * len;
    const SimdFloat4 p5   = pmid - nz * len;

    // clang-format off
    MyDrawLine3D(p0, p2); MyDrawLine3D(p0, p3); MyDrawLine3D(p0, p4); MyDrawLine3D(p0, p5);
    MyDrawLine3D(p1, p2); MyDrawLine3D(p1, p3); MyDrawLine3D(p1, p4); MyDrawLine3D(p1, p5);
    MyDrawLine3D(p2, p3); MyDrawLine3D(p3, p4); MyDrawLine3D(p4, p5); MyDrawLine3D(p5, p2);
    // clang-format on
}

void DrawSkeleton()
{
    const int                numJoints    = g_Ozz->Skeleton.num_joints();
    ozz::span<const int16_t> jointParents = g_Ozz->Skeleton.joint_parents();

    for (int j = 0; j < numJoints; j++)
    {
        // SimpleDrawJoint(j, jointParents[j]);
        DrawJoint(j, jointParents[j]);
    }
}

int main()
{
    g_Ozz = new OzzRuntimeInfo();

    if (!LoadSkeletonData("assets/flair_skeleton.ozz"))
    {
        std::cerr << "Failed to load ozz format skeleton data" << std::endl;
        return 1;
    }

    if (!LoadAnimationData("assets/flair_animation.ozz"))
    {
        std::cerr << "Failed to load ozz format ozz_skin_animation data" << std::endl;
        return 1;
    }

    InitWindow(800, 600, "ozz-animation test");

    float time = 0;

    Camera cam   = {};
    cam.position = {0, 0, -2};
    cam.fovy     = 60;
    cam.up       = {0, 1, 0};
    cam.target   = {0, 0.5, 0};

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        time += deltaTime;

        // Calculate ratio
        float duration = g_Ozz->Animation.duration();
        float ratio    = fmodf(time / duration, 1.0f);

        // Sampling
        ozz::animation::SamplingJob samplingJob;
        samplingJob.animation = &g_Ozz->Animation;
        samplingJob.context   = &g_Ozz->SamplingJobContext;
        samplingJob.ratio     = ratio;
        samplingJob.output    = make_span(g_Ozz->LocalTransforms);
        samplingJob.Run();

        // Local to model
        ozz::animation::LocalToModelJob localToModelJob;
        localToModelJob.skeleton = &g_Ozz->Skeleton;
        localToModelJob.input    = make_span(g_Ozz->LocalTransforms);
        localToModelJob.output   = make_span(g_Ozz->ModelMatrices);
        localToModelJob.Run();

        BeginDrawing();
        ClearBackground({});
        BeginMode3D(cam);
        // Draw skeletons by using 3d lines
        DrawSkeleton();
        EndMode3D();
        EndDrawing();
    }

    CloseWindow();

    delete g_Ozz;
    g_Ozz = nullptr;

    return 0;
}