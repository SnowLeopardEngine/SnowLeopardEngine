// Scene serialization round trip.
//
// Builds a scene, writes it with IO::Serialize (entt snapshot + cereal JSON), reads it back into an empty scene with
// IO::Deserialize and compares entities and component values. Every check prints PASS / FAIL; the exit code is the
// number of failures.

#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#include <cmath>
#include <cstdio>
#include <map>
#include <string>

using namespace SnowLeopardEngine;

namespace
{
    int g_Failures = 0;

    void Check(bool condition, const std::string& name, const std::string& detail = "")
    {
        std::printf("[%s] %-40s %s\n", condition ? "PASS" : "FAIL", name.c_str(), detail.c_str());
        if (!condition)
        {
            ++g_Failures;
        }
    }

    bool Near(const glm::vec3& a, const glm::vec3& b) { return glm::all(glm::lessThan(glm::abs(a - b), glm::vec3(1e-5f))); }

    // Entities of a scene by name.
    std::map<std::string, entt::entity> ByName(entt::registry& registry)
    {
        std::map<std::string, entt::entity> result;
        for (auto [entity, name] : registry.view<NameComponent>().each())
        {
            result[name.Name] = entity;
        }
        return result;
    }
} // namespace

int main() TRY
{
    EngineInitInfo initInfo {};
    initInfo.Window.Title = "Test - SceneSerialization";

    Engine engine;
    if (!engine.Init(initInfo))
    {
        std::fprintf(stderr, "Failed to init engine!\n");
        return 1;
    }

    EngineContext* context = Engine::GetContext();
    auto           source  = context->SceneMngr->CreateScene("Source", true);

    Entity camera                                      = source->CreateEntity("MainCamera");
    camera.GetComponent<TransformComponent>().Position = {1, 2, 3};
    camera.AddComponent<CameraComponent>().FOV         = 75.0f;

    Entity ball                                      = source->CreateEntity("Ball");
    auto&  ballTransform                             = ball.GetComponent<TransformComponent>();
    ballTransform.Position                           = {4, 5, 6};
    ballTransform.Scale                              = {2, 2, 2};
    ball.AddComponent<RigidBodyComponent>().Mass     = 3.5f;
    ball.AddComponent<SphereColliderComponent>(0.75f);
    ball.AddComponent<MeshFilterComponent>().PrimitiveType = MeshPrimitiveType::Sphere;
    ball.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/Blue.dzmaterial";

    // A destroyed entity leaves a hole in the entity storage; the snapshot has to keep identifiers consistent.
    Entity gone = source->CreateEntity("Gone");
    source->DestroyEntity(gone);

    Entity light = source->CreateEntity("Sun");
    light.AddComponent<DirectionalLightComponent>().Intensity = 0.8f;

    const auto path = std::filesystem::temp_directory_path() / "sle_scene_serialization_test.dzscene";
    Check(IO::Serialize(source.get(), path), "serialize writes the file", path.generic_string());

    // A new scene already has a default directional light; LoadFrom replaces the whole registry.
    auto loaded = context->SceneMngr->CreateScene("Loaded", false);
    loaded->LoadFrom(path);

    auto& src = source->GetRegistry();
    auto& dst = loaded->GetRegistry();

    auto srcNames = ByName(src);
    auto dstNames = ByName(dst);
    // MainCamera, Ball, Sun and the scene's default "Directional Light".
    Check(srcNames.size() == 4, "source has 4 live entities", std::to_string(srcNames.size()));
    Check(dstNames.size() == srcNames.size(), "same number of entities", std::to_string(dstNames.size()));
    Check(dstNames.count("Gone") == 0, "destroyed entity stays destroyed");

    for (const auto& [name, srcEntity] : srcNames)
    {
        if (dstNames.count(name) == 0)
        {
            Check(false, "entity '" + name + "' exists after load");
            continue;
        }

        auto dstEntity = dstNames[name];
        Check(src.get<IDComponent>(srcEntity).Id == dst.get<IDComponent>(dstEntity).Id, "'" + name + "' keeps its UUID");
        Check(Near(src.get<TransformComponent>(srcEntity).Position, dst.get<TransformComponent>(dstEntity).Position),
              "'" + name + "' keeps its position");
    }

    if (dstNames.count("Ball") != 0)
    {
        auto ballEntity = dstNames["Ball"];
        Check(Near(dst.get<TransformComponent>(ballEntity).Scale, {2, 2, 2}), "Ball keeps its scale");
        Check(dst.all_of<RigidBodyComponent, SphereColliderComponent, MeshFilterComponent, MeshRendererComponent>(
                  ballEntity),
              "Ball keeps its components");
        Check(dst.get<RigidBodyComponent>(ballEntity).Mass == 3.5f, "Ball keeps its mass");
        Check(dst.get<SphereColliderComponent>(ballEntity).Radius == 0.75f, "Ball keeps its collider radius");
        Check(dst.get<MeshFilterComponent>(ballEntity).PrimitiveType == MeshPrimitiveType::Sphere,
              "Ball keeps its mesh primitive");
        Check(dst.get<MeshRendererComponent>(ballEntity).MaterialFilePath ==
                  std::filesystem::path("Assets/Materials/Next/Blue.dzmaterial"),
              "Ball keeps its material path");
        Check(!dst.any_of<CameraComponent>(ballEntity), "Ball did not gain a camera");
    }

    if (dstNames.count("MainCamera") != 0)
    {
        Check(dst.get<CameraComponent>(dstNames["MainCamera"]).FOV == 75.0f, "camera keeps its FOV");
    }
    if (dstNames.count("Sun") != 0)
    {
        Check(dst.get<DirectionalLightComponent>(dstNames["Sun"]).Intensity == 0.8f, "light keeps its intensity");
    }

    // Loaded scenes keep working: the UUID map is rebuilt and new entities get fresh identifiers.
    if (dstNames.count("Ball") != 0)
    {
        const auto ballId = dst.get<IDComponent>(dstNames["Ball"]).Id;
        Check(loaded->GetEntityWithCoreUUID(ballId).GetName() == "Ball", "UUID lookup works after load");
    }
    Entity extra = loaded->CreateEntity("Extra");
    Check(dst.valid(extra) && ByName(dst).size() == srcNames.size() + 1, "new entity after load");

    std::filesystem::remove(path);
    std::printf("\n%d failure(s)\n", g_Failures);

    engine.Shutdown();
    return g_Failures;
}
CATCH
