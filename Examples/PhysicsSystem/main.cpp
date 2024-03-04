#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsMaterial.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class SphereScript : public NativeScriptInstance
{
public:
    virtual void OnColliderEnter() override
    {
        SNOW_LEOPARD_INFO("[SphereScript] OnColliderEnter");
        DesktopApp::GetInstance()->GetEngine()->GetContext()->AudioSys->Play("sounds/jump.mp3");
    }
 
    virtual void OnTick(float deltaTime) override
    {
        auto& inputSystem = DesktopApp::GetInstance()->GetEngine()->GetContext()->InputSys;
        
        //  Character Controller Test Code
        auto& t = m_OwnerEntity->GetComponent<CharacterControllerComponent>().Ctrl;
        physx::PxControllerFilters filters;
        physx::PxVec3 movement(15 * deltaTime, 0,0);
        t->move(movement, 0.01, deltaTime, filters);
        //------End------------
        
        if (inputSystem->GetKey(KeyCode::Escape))
        {
            DesktopApp::GetInstance()->Quit();
        }
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
        // clang-format off
        cameraComponent.CubemapFilePaths = {
            "Assets/Textures/Skybox001/right.jpg",
            "Assets/Textures/Skybox001/left.jpg",
            "Assets/Textures/Skybox001/top.jpg",
            "Assets/Textures/Skybox001/bottom.jpg",
            "Assets/Textures/Skybox001/front.jpg",
            "Assets/Textures/Skybox001/back.jpg"
        };
        // clang-format on

        camera.AddComponent<FreeMoveCameraControllerComponent>();

        // Create a smooth material
        // https://forum.unity.com/threads/bounciness-1-0-conservation-of-energy-doesnt-work.143472/
        auto smoothMaterial = CreateRef<PhysicsMaterial>(0, 0, 0.98);
        auto normalMaterial = CreateRef<PhysicsMaterial>(0.4, 0.4, 0.4);

        // Create a sphere with RigidBodyComponent & SphereColliderComponent
        Entity sphere = scene->CreateEntity("Sphere");

        auto& sphereTransform    = sphere.GetComponent<TransformComponent>();
        sphereTransform.Position = {5, 15, 0};
        sphereTransform.Scale *= 3;

        sphere.AddComponent<RigidBodyComponent>(1.0f, 0.0f, 0.5f, false);
        sphere.AddComponent<SphereColliderComponent>(normalMaterial);
        auto& sphereMeshFilter                    = sphere.AddComponent<MeshFilterComponent>();
        sphereMeshFilter.PrimitiveType            = MeshPrimitiveType::Sphere;
        auto& sphereMeshRenderer                  = sphere.AddComponent<MeshRendererComponent>();
        sphereMeshRenderer.BaseColor              = {0.4, 0.45, 0.5, 1}; // Metal
        sphereMeshRenderer.UseDiffuse             = true;
        sphereMeshRenderer.DiffuseTextureFilePath = "Assets/Textures/awesomeface.png";

        auto scriptInstance = CreateRef<SphereScript>();
        sphere.AddComponent<NativeScriptingComponent>(scriptInstance);
        
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
        // floorMeshRenderer.BaseColor              = {1, 1, 1, 1}; // Pure White
        // floorMeshRenderer.UseDiffuse             = true;
        // floorMeshRenderer.DiffuseTextureFilePath = "Assets/Textures/CoolGay.png";
        
        sphere.AddComponent<CharacterControllerComponent>();
        
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
        auto& terrainRenderer = terrain.AddComponent<TerrainRendererComponent>();
        // terrainRenderer.BaseColor = {0.6, 1, 0.6, 1}; // Light Green
        terrainRenderer.UseDiffuse             = true;
        terrainRenderer.DiffuseTextureFilePath = "Assets/Textures/CoolGay.png";
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
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