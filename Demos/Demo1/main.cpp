#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class EscScript : public NativeScriptInstance
{
public:
    virtual void OnTick(float deltaTime) override
    {
        if (m_EngineContext->InputSys->GetKey(KeyCode::Escape))
        {
            DesktopApp::GetInstance()->Quit();
        }
    }
};

class MyCharacterController : public NativeScriptInstance
{
public:
    virtual void OnLoad() override { m_Controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>(); }

    virtual void OnTick(float deltaTime) override
    {
        if (m_EngineContext->InputSys->GetKey(KeyCode::Up))
        {
            m_EngineContext->PhysicsSys->Move(m_Controller, glm::vec3(0, 0, 0.1), deltaTime);
        }

        if (m_EngineContext->InputSys->GetKey(KeyCode::Down))
        {
            m_EngineContext->PhysicsSys->Move(m_Controller, glm::vec3(0, 0, -0.1), deltaTime);
        }
    }

private:
    CharacterControllerComponent m_Controller;
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

        // TODO: Switch to 3rd Person Follow Camera Controller
        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(CreateRef<EscScript>());

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform          = floor.GetComponent<TransformComponent>();
        floorTransform.Scale          = {50, 1, 50};
        auto& floorMeshFilter         = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer       = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.BaseColor   = {0.4, 0.1, 0.1, 1};

        // Create a character
        Entity character                = scene->CreateEntity("Character");
        auto&  characterTransform       = character.GetComponent<TransformComponent>();
        characterTransform.Position.y   = 0.6;
        characterTransform.Scale        = {10, 10, 10};
        auto& characterMeshFilter       = character.AddComponent<MeshFilterComponent>();
        characterMeshFilter.FilePath    = "Assets/Models/Vampire/Vampire_Idle.dae";
        auto& characterMeshRenderer     = character.AddComponent<MeshRendererComponent>();
        characterMeshRenderer.BaseColor = {0.1, 0.1, 0.7, 1};
        character.AddComponent<AnimatorComponent>();
        character.AddComponent<CharacterControllerComponent>();
        character.AddComponent<NativeScriptingComponent>(CreateRef<MyCharacterController>());
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - AnimationSystem";
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