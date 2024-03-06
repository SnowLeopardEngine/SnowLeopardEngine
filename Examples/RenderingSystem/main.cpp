#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class EscScript : public NativeScriptInstance
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
        auto scene = m_EngineContext->SceneMngr->CreateScene("RenderingSystem", true);

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
        camera.AddComponent<NativeScriptingComponent>(CreateRef<EscScript>());

        std::ifstream            redFile("Assets/Materials/Red.dzmaterial");
        cereal::JSONInputArchive archiveRed(redFile);
        auto                     red = CreateRef<DzMaterial>();
        archiveRed(*red);

        std::ifstream            greenFile("Assets/Materials/Green.dzmaterial");
        cereal::JSONInputArchive archiveGreen(greenFile);
        auto                     green = CreateRef<DzMaterial>();
        archiveGreen(*green);

        std::ifstream            blueFile("Assets/Materials/Blue.dzmaterial");
        cereal::JSONInputArchive archiveBlue(blueFile);
        auto                     blue = CreateRef<DzMaterial>();
        archiveBlue(*blue);

        // Create cubes to test shadow
        Entity cube1            = scene->CreateEntity("Cube1");
        auto&  cubeTransform1   = cube1.GetComponent<TransformComponent>();
        cubeTransform1.Position = {0, 10, 0};
        cubeTransform1.SetRotationEuler(glm::vec3(45, 45, 45));
        auto& cubeMeshFilter1         = cube1.AddComponent<MeshFilterComponent>();
        cubeMeshFilter1.PrimitiveType = MeshPrimitiveType::Cube;
        auto& cubeMeshRenderer1       = cube1.AddComponent<MeshRendererComponent>();
        // cubeMeshRenderer1.BaseColor   = {1, 0, 0, 1};
        cubeMeshRenderer1.Material = red;

        Entity cube2            = scene->CreateEntity("Cube2");
        auto&  cubeTransform2   = cube2.GetComponent<TransformComponent>();
        cubeTransform2.Position = {-3, 5, -5};
        cubeTransform2.SetRotationEuler(glm::vec3(135, 135, 135));
        cubeTransform2.Scale          = {2, 2, 2};
        auto& cubeMeshFilter2         = cube2.AddComponent<MeshFilterComponent>();
        cubeMeshFilter2.PrimitiveType = MeshPrimitiveType::Cube;
        auto& cubeMeshRenderer2       = cube2.AddComponent<MeshRendererComponent>();
        // cubeMeshRenderer2.BaseColor   = {0, 1, 0, 1};
        cubeMeshRenderer2.Material = green;

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform          = floor.GetComponent<TransformComponent>();
        floorTransform.Scale          = {50, 1, 50};
        auto& floorMeshFilter         = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer       = floor.AddComponent<MeshRendererComponent>();
        // floorMeshRenderer.BaseColor   = {1, 1, 1, 1}; // Pure White
        floorMeshRenderer.Material = blue;
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - RenderingSystem";
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