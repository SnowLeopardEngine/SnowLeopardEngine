// Skeletal animation regression test.
//
// Loads the Vampire model with two ozz clips (Dancing, Walking), ticks the engine with a fixed delta time and checks the
// CPU-skinned vertices: the mesh leaves its bind pose, keeps moving, and a parameter-driven transition blends to the
// other clip and back without a jump. Exit code = number of failures.

#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

using namespace SnowLeopardEngine;

namespace
{
    int g_Failures = 0;

    void Check(bool condition, const std::string& name, const std::string& detail = "")
    {
        std::printf("[%s] %-48s %s\n", condition ? "PASS" : "FAIL", name.c_str(), detail.c_str());
        if (!condition)
        {
            ++g_Failures;
        }
    }

    std::vector<glm::vec3> Positions(const MeshItem& mesh)
    {
        std::vector<glm::vec3> result;
        result.reserve(mesh.Data.Vertices.size());
        for (const auto& vertex : mesh.Data.Vertices)
        {
            result.push_back(vertex.Position);
        }
        return result;
    }

    // Mean vertex displacement between two snapshots of the same mesh.
    float MeanDistance(const std::vector<glm::vec3>& a, const std::vector<glm::vec3>& b)
    {
        if (a.empty() || a.size() != b.size())
        {
            return -1.0f;
        }
        double sum = 0.0;
        for (size_t i = 0; i < a.size(); ++i)
        {
            sum += glm::length(a[i] - b[i]);
        }
        return static_cast<float>(sum / static_cast<double>(a.size()));
    }

    std::string Num(float value)
    {
        char buffer[32];
        std::snprintf(buffer, sizeof(buffer), "%.4f", value);
        return buffer;
    }
} // namespace

int main() TRY
{
    constexpr float kDeltaTime = 1.0f / 60.0f;
    constexpr float kBlendTime = 0.3f;

    EngineInitInfo initInfo {};
    initInfo.Window.Title = "Test - AnimationPlayback";

    Engine engine;
    if (!engine.Init(initInfo))
    {
        std::fprintf(stderr, "Failed to init engine!\n");
        return 1;
    }

    auto model = CreateRef<Model>();
    {
        OzzModelLoadConfig config = {};
        config.OzzMeshPath        = "Assets/Models/Vampire/mesh.ozz";
        config.OzzSkeletonPath    = "Assets/Models/Vampire/skeleton.ozz";
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/animation_Dancing.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Walking.ozz");
        Check(OzzModelLoader::Load(config, model.get()), "Vampire model loads");
    }
    if (model->AnimationClips.size() != 2 || model->AnimationClips[0]->MeshPtr == nullptr)
    {
        Check(false, "two clips with a skinned mesh", std::to_string(model->AnimationClips.size()));
        engine.Shutdown();
        return g_Failures;
    }
    auto  dance = model->AnimationClips[0];
    auto  walk  = model->AnimationClips[1];
    auto& mesh  = *dance->MeshPtr;
    Check(mesh.Skinned() && walk->MeshPtr == dance->MeshPtr, "both clips skin the same mesh");
    const auto bindPose = Positions(mesh);

    auto   scene     = g_EngineContext->SceneMngr->CreateScene("AnimationPlayback", true);
    Entity character = scene->CreateEntity("Character");
    character.AddComponent<MeshFilterComponent>().Meshes = ShareMeshes(model);
    character.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/Vampire.dzmaterial";

    // No SetEntryAnimationClip: the first registered clip has to become the entry clip.
    auto controller = CreateRef<AnimatorController>();
    controller->RegisterAnimationClip(dance);
    controller->RegisterAnimationClip(walk);
    controller->RegisterParameters("Walking", false);
    controller->RegisterTransition(dance, walk, kBlendTime)->SetConditions("Walking", ConditionOperator::Equal, true);
    controller->RegisterTransition(walk, dance, kBlendTime)->SetConditions("Walking", ConditionOperator::Equal, false);
    auto& animator = character.AddComponent<AnimatorComponent>().CurrentAnimator;
    animator.SetController(controller);
    Check(controller->GetAnimationClip() == dance, "first registered clip is the entry clip");

    engine.PostInit();
    auto tick = [&engine](int frames) {
        for (int i = 0; i < frames; ++i)
        {
            engine.TickOneFrame(kDeltaTime);
        }
    };

    tick(30);
    const auto  posed        = Positions(mesh);
    const float fromBindPose = MeanDistance(bindPose, posed);
    Check(fromBindPose > 0.01f, "mesh leaves the bind pose", "mean offset " + Num(fromBindPose));

    // Typical per-frame motion while dancing, used as the yardstick for jumps.
    float maxStep = 0.0f;
    auto  last    = posed;
    for (int i = 0; i < 30; ++i)
    {
        tick(1);
        auto now = Positions(mesh);
        maxStep  = std::max(maxStep, MeanDistance(last, now));
        last     = std::move(now);
    }
    Check(maxStep > 0.0f, "dancing keeps moving", "max step " + Num(maxStep));

    // Dance -> Walk: the blend starts from the current dance pose (no jump) and ends on the walk clip.
    animator.SetBoolean("Walking", true);
    float blendStep = 0.0f;
    for (int i = 0; i < 5; ++i)
    {
        tick(1);
        auto now  = Positions(mesh);
        blendStep = std::max(blendStep, MeanDistance(last, now));
        last      = std::move(now);
    }
    Check(blendStep < maxStep * 3.0f + 1e-4f,
          "blend into Walking starts without a jump",
          "max step " + Num(blendStep) + " vs dance " + Num(maxStep));
    tick(static_cast<int>(kBlendTime / kDeltaTime) + 5);
    Check(controller->GetAnimationClip() == walk, "transition reaches Walking");

    auto walking = Positions(mesh);
    tick(10);
    Check(MeanDistance(walking, Positions(mesh)) > 0.0f, "walking keeps moving");

    // Walk -> Dance.
    animator.SetBoolean("Walking", false);
    tick(static_cast<int>(kBlendTime / kDeltaTime) + 10);
    Check(controller->GetAnimationClip() == dance, "transition back to Dancing");

    std::printf("\n%d failure(s)\n", g_Failures);

    scene.reset();
    engine.Shutdown();
    return g_Failures;
}
CATCH
