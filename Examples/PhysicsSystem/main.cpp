#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsMaterial.h"
#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "glm/fwd.hpp"
#include <SnowLeopardEngine/Core/Time/Time.h>
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class SphereScript : public NativeScriptInstance
{
public:
    virtual void OnColliderEnter() override
    {
        SNOW_LEOPARD_INFO("[SphereScript] OnColliderEnter");
        g_EngineContext->AudioSys->Play("sounds/jump.mp3");
    }

    virtual void OnFixedTick() override
    {
        auto& inputSystem = g_EngineContext->InputSys;

        if (inputSystem->GetKey(KeyCode::Escape))
        {
            DesktopApp::GetInstance()->Quit();
        }

        auto& controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>();
        auto& rigider    = m_OwnerEntity->GetComponent<RigidBodyComponent>();
        g_EngineContext->PhysicsSys->Move(controller, glm::vec3(-0.1f, -0.1f, 0), Time::FixedDeltaTime);
        g_EngineContext->PhysicsSys->AddForce(rigider, glm::vec3(-5.f, -5.f, 0));
    }
};

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_EngineContext = DesktopApp::GetInstance()->GetEngine()->GetContext();

        // Hide mouse cursor
        m_EngineContext->WindowSys->SetHideCursor(true);

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("PhysicsSystem", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox
        cameraComponent.SkyboxMaterialFilePath             = "Assets/Materials/Skybox001.dzmaterial";

        camera.AddComponent<FreeMoveCameraControllerComponent>();

        // Create a smooth material
        // https://forum.unity.com/threads/bounciness-1-0-conservation-of-energy-doesnt-work.143472/
        auto smoothMaterial = CreateRef<PhysicsMaterial>(0, 0, 0.98);
        auto normalMaterial = CreateRef<PhysicsMaterial>(0.4, 0.4, 0.4);

        // Create a sphere with RigidBodyComponent & SphereColliderComponent
        Entity sphere = scene->CreateEntity("Sphere");

        auto& sphereTransform    = sphere.GetComponent<TransformComponent>();
        sphereTransform.Position = {0, 15, 0};
        sphereTransform.Scale *= 3;

        sphere.AddComponent<RigidBodyComponent>();
        sphere.AddComponent<SphereColliderComponent>(normalMaterial);
        sphere.AddComponent<CharacterControllerComponent>();

        auto& sphereMeshFilter              = sphere.AddComponent<MeshFilterComponent>();
        sphereMeshFilter.PrimitiveType      = MeshPrimitiveType::Sphere;
        auto& sphereMeshRenderer            = sphere.AddComponent<MeshRendererComponent>();
        sphereMeshRenderer.MaterialFilePath = "Assets/Materials/Blue.dzmaterial";

        sphere.AddComponent<NativeScriptingComponent>("SphereScript");

        // create a testSphere to test MeshColliderComponent
        Entity testSphere            = scene->CreateEntity("TestSphere");
        auto&  testSphereTransform   = testSphere.GetComponent<TransformComponent>();
        testSphereTransform.Position = {5, 15, 0};
        testSphereTransform.Scale *= 3;
        testSphere.AddComponent<RigidBodyComponent>(1.0f, 0.0f, 0.5f, false);
        auto& testSphereMeshFilter              = testSphere.AddComponent<MeshFilterComponent>();
        testSphereMeshFilter.PrimitiveType      = MeshPrimitiveType::Sphere;
        auto& testSphereMeshCollider            = testSphere.AddComponent<MeshColliderComponent>();
        auto& testSphereMeshRenderer            = testSphere.AddComponent<MeshRendererComponent>();
        testSphereMeshRenderer.MaterialFilePath = "Assets/Materials/Red.dzmaterial";

        // // Create a floor with RigidBodyComponent & BoxColliderComponent
        // Entity floor = scene->CreateEntity("Floor");

        // auto& floorTransform = floor.GetComponent<TransformComponent>();
        // floorTransform.Scale = {50, 1, 50};

        // // set it to static, so that rigidBody will be static.
        // floor.GetComponent<EntityStatusComponent>().IsStatic = true;
        // floor.AddComponent<RigidBodyComponent>();
        // floor.AddComponent<BoxColliderComponent>(smoothMaterial);
        // auto& floorMeshFilter                    = floor.AddComponent<MeshFilterComponent>();
        // floorMeshFilter.PrimitiveType            = MeshPrimitiveType::Cube;
        // auto& floorMeshRenderer                  = floor.AddComponent<MeshRendererComponent>();

        // Create a terrain
        int    heightMapWidth                               = 100;
        int    heightMapHeight                              = 100;
        float  xScale                                       = 3;
        float  yScale                                       = 3;
        float  zScale                                       = 8;
        Entity terrain                                      = scene->CreateEntity("Terrain");
        terrain.GetComponent<TransformComponent>().Position = {
            -heightMapWidth * 0.5f * xScale, 0, -heightMapHeight * 0.5f * zScale}; // fix center
        auto& terrainComponent = terrain.AddComponent<TerrainComponent>();
        terrainComponent.TerrainHeightMap =
            Utils::GenerateWaveHeightMap(heightMapWidth, heightMapHeight); // create a 100 x 100 height map
        terrainComponent.XScale = xScale;
        terrainComponent.YScale = yScale;
        terrainComponent.ZScale = zScale;
        terrain.AddComponent<TerrainColliderComponent>(normalMaterial);
        auto& terrainRenderer    = terrain.AddComponent<TerrainRendererComponent>();
        terrainRenderer.Material = DzMaterial::LoadFromPath("Assets/Materials/CoolGay.dzmaterial");
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
    REGISTER_TYPE(SphereScript);

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - PhysicsSystem";
    DesktopApp app(argc, argv);

    if (!app.Init(initInfo))
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    // add a custom lifeTimeComponent to the engine
    app.GetEngine()->AddLifeTimeComponent(CreateRef<CustomLifeTime>());

    if (!app.PostInit())
    {
        std::cerr << "Failed to post initialize the application!" << std::endl;
        return 1;
    }

    app.Run();

    return 0;
}