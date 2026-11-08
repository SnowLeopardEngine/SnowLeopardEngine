// Scene serialization round trip.
//
// Builds a scene, writes it with IO::Serialize (entt snapshot + cereal JSON), reads it back into an empty scene with
// IO::Deserialize and compares entities and component values; then the scene graph the editor edits (reparenting with
// world transforms kept, cycles refused, duplication with children, deletion of a subtree, a hierarchy that survives
// a save, the play-mode copy). Every check prints PASS / FAIL; the exit code is the number of failures.

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

    // ---- scene graph (editor Hierarchy)
    {
        auto   graph  = context->SceneMngr->CreateScene("Graph", false);
        Entity parent = graph->CreateEntity("Parent");
        Entity child  = graph->CreateEntity("Child");
        parent.GetComponent<TransformComponent>().Position = {10, 0, 0};
        parent.GetComponent<TransformComponent>().SetRotationEuler({0, 90, 0});
        parent.GetComponent<TransformComponent>().Scale = {2, 2, 2};
        child.GetComponent<TransformComponent>().Position = {1, 2, 3};

        Check(graph->SetParent(child, parent, true), "SetParent succeeds");
        Check(child.GetParentUUID() == parent.GetCoreUUID() && parent.GetChildrenUUIDs().size() == 1, "parent and child are linked");
        Check(Near(glm::vec3(graph->GetWorldTransform(child)[3]), {1, 2, 3}), "keepWorld keeps the child where it was");
        Check(!Near(child.GetComponent<TransformComponent>().Position, {1, 2, 3}), "its local transform is rewritten");

        parent.GetComponent<TransformComponent>().Position = {20, 0, 0};
        Check(Near(glm::vec3(graph->GetWorldTransform(child)[3]), {11, 2, 3}), "the child follows its parent");
        Check(!graph->SetParent(parent, child, true), "a parent cannot become its child's child");

        Entity grandChild = graph->CreateEntity("GrandChild");
        graph->SetParent(grandChild, child, false);
        Check(graph->IsDescendantOf(grandChild, parent), "grandchild is a descendant of the root");

        const auto rootsBefore = graph->GetRootEntities().size();
        Entity     copy        = graph->DuplicateEntity(parent);
        Check(copy && copy.GetCoreUUID() != parent.GetCoreUUID() && copy.GetChildrenUUIDs().size() == 1, "duplicate copies the subtree");
        Check(graph->GetRootEntities().size() == rootsBefore + 1, "the duplicate is a sibling (a new root)");
        Entity copiedChild = graph->GetEntityWithCoreUUID(copy.GetChildrenUUIDs()[0]);
        Check(copiedChild && copiedChild.GetCoreUUID() != child.GetCoreUUID() && copiedChild.GetChildrenUUIDs().size() == 1 &&
                  Near(glm::vec3(graph->GetWorldTransform(copiedChild)[3]), glm::vec3(graph->GetWorldTransform(child)[3])),
              "copied children keep their transforms");

        // Save and reload: the hierarchy is part of the scene file.
        const auto graphPath = std::filesystem::temp_directory_path() / "sle_scene_graph_test.dzscene";
        graph->SaveTo(graphPath);
        auto reloaded = context->SceneMngr->CreateScene("GraphLoaded", true);
        reloaded->LoadFrom(graphPath);
        std::filesystem::remove(graphPath);
        Entity reloadedChild = reloaded->GetEntityWithCoreUUID(child.GetCoreUUID());
        Check(reloadedChild && reloadedChild.GetParentUUID() == parent.GetCoreUUID() &&
                  Near(glm::vec3(reloaded->GetWorldTransform(reloadedChild)[3]), {11, 2, 3}),
              "hierarchy survives save / load");

        // Play mode copies the scene: same UUIDs, same order of roots.
        auto played = LogicScene::Copy(graph);
        auto names  = [](const std::vector<Entity>& entities) {
            std::string list;
            for (auto entity : entities)
                list += entity.GetName() + ",";
            return list;
        };
        Check(names(played->GetRootEntities()) == names(graph->GetRootEntities()), "play copy lists roots in the same order",
              names(played->GetRootEntities()));

        // Deleting a parent deletes its subtree; unknown ids give a null entity instead of throwing.
        const auto childId = child.GetCoreUUID(), grandChildId = grandChild.GetCoreUUID();
        graph->DestroyEntity(parent);
        Check(!graph->GetEntityWithCoreUUID(childId) && !graph->GetEntityWithCoreUUID(grandChildId),
              "deleting a parent deletes its children");
        Check(!graph->GetEntityWithCoreUUID(CoreUUIDHelper::CreateStandardUUID()), "unknown UUID gives a null entity");
    }

    std::printf("\n%d failure(s)\n", g_Failures);

    engine.Shutdown();
    return g_Failures;
}
CATCH
