// Engine-level physics regression test.
//
// Builds one scene with a set of small, spatially separated scenarios, steps the engine's fixed tick a known number of
// times (no dependence on wall-clock time) and checks where bodies end up. Every check prints PASS / FAIL; the exit
// code is the number of failures.

#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsMaterial.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsSystem.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#include <cmath>
#include <cstdio>
#include <string>

using namespace SnowLeopardEngine;

namespace
{
    int g_Failures = 0;
    int g_ContactEnterCount = 0;

    void Check(bool condition, const std::string& name, const std::string& detail)
    {
        std::printf("[%s] %-34s %s\n", condition ? "PASS" : "FAIL", name.c_str(), detail.c_str());
        if (!condition)
        {
            ++g_Failures;
        }
    }

    std::string Vec(const glm::vec3& v)
    {
        char buffer[96];
        std::snprintf(buffer, sizeof(buffer), "(%.3f, %.3f, %.3f)", v.x, v.y, v.z);
        return buffer;
    }

    bool Near(float value, float expected, float tolerance) { return std::fabs(value - expected) <= tolerance; }

    // Counts OnColliderEnter calls for the entity it is attached to.
    class ContactCounter : public NativeScriptInstance
    {
    public:
        void OnColliderEnter() override { ++g_ContactEnterCount; }
    };

    Entity MakeStaticBox(const Ref<LogicScene>& scene, const std::string& name, glm::vec3 position, glm::vec3 size)
    {
        Entity entity                                      = scene->CreateEntity(name);
        entity.GetComponent<TransformComponent>().Position = position;
        entity.GetComponent<EntityStatusComponent>().IsStatic = true;
        entity.AddComponent<RigidBodyComponent>();
        entity.AddComponent<BoxColliderComponent>(size);
        return entity;
    }

    Entity MakeDynamic(const Ref<LogicScene>& scene, const std::string& name, glm::vec3 position)
    {
        Entity entity                                      = scene->CreateEntity(name);
        entity.GetComponent<TransformComponent>().Position = position;
        entity.AddComponent<RigidBodyComponent>();
        return entity;
    }

    glm::vec3 PositionOf(Entity entity) { return entity.GetComponent<TransformComponent>().Position; }
} // namespace

int main() TRY
{
    REGISTER_TYPE(ContactCounter);

    EngineInitInfo initInfo {};
    initInfo.Window.Title = "Test - PhysicsScenarios";

    Engine engine;
    if (!engine.Init(initInfo))
    {
        std::fprintf(stderr, "Failed to init engine!\n");
        return 1;
    }

    EngineContext* context = Engine::GetContext();
    auto           scene   = context->SceneMngr->CreateScene("PhysicsScenarios", true);

    Entity camera                                      = scene->CreateEntity("MainCamera");
    camera.GetComponent<TransformComponent>().Position = {0, 10, 60};
    camera.AddComponent<CameraComponent>();

    // A shared static floor; its top face is at y = 0.5.
    MakeStaticBox(scene, "Floor", {0, 0, 0}, {300, 1, 40});
    constexpr float floorTop = 0.5f;

    // 1. Sphere collider comes to rest on a box. Also counts collision-enter callbacks.
    Entity sphere = MakeDynamic(scene, "SphereRest", {-120, 5, 0});
    sphere.AddComponent<SphereColliderComponent>(0.5f);
    sphere.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(ContactCounter));

    // 2. Box collider.
    Entity box = MakeDynamic(scene, "BoxRest", {-100, 5, 0});
    box.AddComponent<BoxColliderComponent>(glm::vec3(1, 1, 1));

    // 3. Capsule collider (Y-up; Height is the cylinder part).
    Entity capsule = MakeDynamic(scene, "CapsuleRest", {-80, 5, 0});
    capsule.AddComponent<CapsuleColliderComponent>(0.5f, 1.0f);

    // 4. Convex mesh collider from a primitive sphere mesh (radius 0.5).
    Entity meshBody = MakeDynamic(scene, "MeshRest", {-60, 5, 0});
    auto&  meshFilter        = meshBody.AddComponent<MeshFilterComponent>();
    meshFilter.PrimitiveType = MeshPrimitiveType::Sphere;
    meshBody.AddComponent<MeshColliderComponent>();

    // 5. A static rigid body stays where it is.
    Entity staticBox = MakeStaticBox(scene, "StaticBox", {-40, 3, 0}, {1, 1, 1});

    // 6. A trigger (sensor) does not stop a falling body.
    Entity trigger                                        = scene->CreateEntity("Trigger");
    trigger.GetComponent<TransformComponent>().Position   = {-20, 3, 0};
    trigger.GetComponent<EntityStatusComponent>().IsStatic = true;
    trigger.AddComponent<RigidBodyComponent>();
    trigger.AddComponent<SphereColliderComponent>(2.0f, nullptr, true);
    Entity throughTrigger = MakeDynamic(scene, "ThroughTrigger", {-20, 8, 0});
    throughTrigger.AddComponent<SphereColliderComponent>(0.5f);

    // 7. Character controller walking on the floor (displacement-based Move, gravity applied by the caller).
    Entity character                                      = scene->CreateEntity("Character");
    character.GetComponent<TransformComponent>().Position = {0, 2, 0};
    auto& controller  = character.AddComponent<CharacterControllerComponent>();
    controller.Height = 1.0f;
    controller.Radius = 0.5f;
    controller.Offset = {0, 0, 0};

    // 8. AddForce on a frictionless box.
    Entity pushed = MakeDynamic(scene, "Pushed", {20, 1.0f, 0});
    pushed.AddComponent<BoxColliderComponent>(glm::vec3(1, 1, 1));

    // 9. A body destroyed at runtime.
    Entity doomed = MakeDynamic(scene, "Doomed", {40, 5, 0});
    doomed.AddComponent<SphereColliderComponent>(0.5f);

    // 10. RigidBody + CharacterController on one entity: the controller drives it, the rigid body is ignored.
    Entity hybrid                                      = scene->CreateEntity("Hybrid");
    hybrid.GetComponent<TransformComponent>().Position = {80, 2, 10};
    hybrid.AddComponent<RigidBodyComponent>();
    hybrid.AddComponent<SphereColliderComponent>(0.5f);
    auto& hybridController  = hybrid.AddComponent<CharacterControllerComponent>();
    hybridController.Offset = {0, 0, 0};

    // 11. Height field: a ramp along X (height = 0.5 * x) on a separate, floor-free area.
    constexpr int   terrainSize = 32;
    const glm::vec3 terrainOrigin {100, 0, -100};
    Entity          terrain                              = scene->CreateEntity("Terrain");
    terrain.GetComponent<TransformComponent>().Position  = terrainOrigin;
    auto& terrainComponent                               = terrain.AddComponent<TerrainComponent>();
    HeightMap heightMap                                  = Utils::GenerateBlankHeightMap(terrainSize, terrainSize);
    // The terrain mesh and the collider both place sample Get(row, column) == Data[column][row] at x = column,
    // z = row, so Data[x][z] is the height at local (x, z).
    for (int x = 0; x < terrainSize; ++x)
    {
        for (int z = 0; z < terrainSize; ++z)
        {
            heightMap.Data[x][z] = 0.5f * static_cast<float>(x);
        }
    }
    terrainComponent.TerrainHeightMap = heightMap;
    terrain.AddComponent<TerrainColliderComponent>(nullptr);

    // Load the scene: builds meshes and cooks the physics world.
    if (!engine.PostInit())
    {
        std::fprintf(stderr, "PostInit failed\n");
        return 1;
    }

    auto&           physics = context->PhysicsSys;
    const float     dt      = Time::FixedDeltaTime;
    constexpr int   ticks   = 300; // 6 s at 50 Hz
    Entity          spawned;
    glm::vec3       doomedLastPosition {};

    for (int tick = 0; tick < ticks; ++tick)
    {
        // Character: walk +X at 1 m/s, with gravity as a displacement like GameDemo.
        physics->Move(character.GetComponent<CharacterControllerComponent>(), glm::vec3(1.0f, -9.8f, 0.0f) * dt, dt);

        // Push for one second.
        if (tick >= 50 && tick < 100)
        {
            physics->AddForce(pushed.GetComponent<RigidBodyComponent>(), {20, 0, 0});
        }

        // Runtime spawn, with a mesh and a material like the PhysicsSystem example's Space key.
        if (tick == 10)
        {
            spawned = MakeDynamic(scene, "Spawned", {60, 5, 0});
            spawned.AddComponent<SphereColliderComponent>(0.5f);
            auto& spawnedMeshFilter              = spawned.AddComponent<MeshFilterComponent>();
            spawnedMeshFilter.PrimitiveType      = MeshPrimitiveType::Sphere;
            auto& spawnedRenderer                = spawned.AddComponent<MeshRendererComponent>();
            spawnedRenderer.MaterialFilePath     = "Assets/Materials/Next/Blue.dzmaterial";
            scene->TriggerEntityCreate(spawned);
        }

        // The hybrid entity is moved by its controller only.
        physics->Move(hybrid.GetComponent<CharacterControllerComponent>(), glm::vec3(0.0f, -9.8f, 0.0f) * dt, dt);

        // Runtime destroy.
        if (tick == 150)
        {
            doomedLastPosition = PositionOf(doomed);
            scene->TriggerEntityDestroy(doomed);
            scene->DestroyEntity(doomed);
        }

        engine.FixedTickOneFrame();
        engine.TickOneFrame(dt); // also renders, so runtime-spawned entities go through the renderer
    }

    std::printf("\n== Results after %d fixed ticks (%.2f s) ==\n", ticks, ticks * dt);

    const glm::vec3 spherePos = PositionOf(sphere);
    Check(Near(spherePos.y, floorTop + 0.5f, 0.05f), "sphere rests on box", Vec(spherePos));
    Check(g_ContactEnterCount >= 1 && g_ContactEnterCount <= 3,
          "OnColliderEnter once per touch",
          std::to_string(g_ContactEnterCount) + " call(s)");

    const glm::vec3 boxPos = PositionOf(box);
    Check(Near(boxPos.y, floorTop + 0.5f, 0.05f), "box rests on box", Vec(boxPos));

    const glm::vec3 capsulePos = PositionOf(capsule);
    // Upright: center at floor + radius + half height (1.5 above the floor top); lying down: radius (0.5).
    Check(capsulePos.y > floorTop + 0.45f && capsulePos.y < floorTop + 1.05f,
          "capsule rests (upright or lying)",
          Vec(capsulePos));

    const glm::vec3 meshPos = PositionOf(meshBody);
    Check(Near(meshPos.y, floorTop + 0.5f, 0.1f), "convex mesh rests on box", Vec(meshPos));

    const glm::vec3 staticPos = PositionOf(staticBox);
    Check(Near(staticPos.y, 3.0f, 1e-4f), "static body does not move", Vec(staticPos));

    const glm::vec3 throughPos = PositionOf(throughTrigger);
    Check(Near(throughPos.y, floorTop + 0.5f, 0.05f), "body falls through a trigger", Vec(throughPos));

    const glm::vec3 characterPos = PositionOf(character);
    // Center sits Height/2 + Radius above the floor; it walked 1 m/s for 6 s.
    Check(Near(characterPos.y, floorTop + 1.0f, 0.1f), "character stands on the floor", Vec(characterPos));
    Check(Near(characterPos.x, ticks * dt, 0.3f), "character walks with Move()", Vec(characterPos));
    Check(physics->SimpleRaycast(characterPos - glm::vec3(0, controller.Height + 0.1f, 0), {0, -1, 0}, 0.15f),
          "grounding ray (GameDemo style) hits",
          "");

    OverlapInfo overlapCharacter;
    physics->OverlapSphere(characterPos, 0.25f, overlapCharacter);
    bool foundCharacter = false;
    for (auto& entity : overlapCharacter.OverlappedEntities)
    {
        foundCharacter |= entity.GetName() == "Character";
    }
    Check(foundCharacter, "overlap finds the character", "");

    const glm::vec3 pushedPos = PositionOf(pushed);
    // 20 N on 1 kg for 1 s on a frictionless floor: v = 20 m/s, so it travels far.
    Check(pushedPos.x > 20.0f + 10.0f, "AddForce moves a body", Vec(pushedPos));

    const glm::vec3 spawnedPos = PositionOf(spawned);
    Check(Near(spawnedPos.y, floorTop + 0.5f, 0.05f), "runtime-spawned body simulates", Vec(spawnedPos));
    Check(spawned.GetComponent<MeshFilterComponent>().Meshes != nullptr &&
              spawned.GetComponent<MeshRendererComponent>().Mat != nullptr,
          "runtime-spawned entity gets mesh + material",
          "");

    const glm::vec3 hybridPos = PositionOf(hybrid);
    Check(hybrid.GetComponent<RigidBodyComponent>().InternalBody == InvalidPhysicsBody &&
              Near(hybridPos.y, floorTop + 1.0f, 0.1f),
          "RigidBody ignored under a controller",
          Vec(hybridPos));

    OverlapInfo overlapDoomed;
    physics->OverlapSphere(doomedLastPosition, 0.6f, overlapDoomed);
    bool foundDoomed = false;
    for (auto& entity : overlapDoomed.OverlappedEntities)
    {
        foundDoomed |= entity.GetName() == "Floor" ? false : true;
    }
    Check(!foundDoomed, "destroyed body is gone", Vec(doomedLastPosition));

    OverlapInfo overlapSphere;
    physics->OverlapSphere(spherePos, 0.6f, overlapSphere);
    bool foundSphere = false;
    for (auto& entity : overlapSphere.OverlappedEntities)
    {
        foundSphere |= entity.GetName() == "SphereRest";
    }
    Check(foundSphere, "overlap finds a rigid body", "");

    Check(physics->SimpleRaycast({-120, 3, 0}, {0, -1, 0}, 5.0f), "ray hits the sphere / floor", "");
    Check(!physics->SimpleRaycast({-130, 3, 0}, {0, -1, 0}, 1.0f), "short ray in empty space misses", "");

    // Height field: bracket the surface with two rays at local x = 10 and x = 20 (expected heights 5 and 10).
    for (float localX : {10.0f, 20.0f})
    {
        const float     expected = 0.5f * localX;
        const glm::vec3 above    = terrainOrigin + glm::vec3(localX, 30.0f, 5.0f);
        const float     toSurface = 30.0f - expected;
        const bool      reaches   = physics->SimpleRaycast(above, {0, -1, 0}, toSurface + 0.1f);
        const bool      stopsShort = !physics->SimpleRaycast(above, {0, -1, 0}, toSurface - 0.1f);
        Check(reaches && stopsShort,
              "height field matches the mesh at x=" + std::to_string(static_cast<int>(localX)),
              "expected height " + std::to_string(expected));
    }

    std::printf("\n%d failure(s)\n", g_Failures);

    engine.Shutdown();
    return g_Failures;
}
CATCH
