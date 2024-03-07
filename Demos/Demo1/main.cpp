#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Asset/Loaders/ModelLoader.h"
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
    virtual void OnLoad() override
    {
        m_Controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>();
        m_Animator   = m_OwnerEntity->GetComponent<AnimatorComponent>();
    }

    virtual void OnTick(float deltaTime) override
    {
        if (m_EngineContext->InputSys->GetKey(KeyCode::W))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[1];
            m_Speed.z                  = 0.1;
        }

        if (m_EngineContext->InputSys->GetKeyUp(KeyCode::W))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[0];
            m_Speed.z                  = 0.0;
        }

        if (m_EngineContext->InputSys->GetKey(KeyCode::S))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[1];
            m_Speed.z                  = -0.1;
        }

        if (m_EngineContext->InputSys->GetKeyUp(KeyCode::S))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[0];
            m_Speed.z                  = 0;
        }

        if (m_EngineContext->InputSys->GetKey(KeyCode::A))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[1];
            m_Speed.x                  = 0.1;
        }

        if (m_EngineContext->InputSys->GetKeyUp(KeyCode::A))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[0];
            m_Speed.x                  = 0;
        }

        if (m_EngineContext->InputSys->GetKey(KeyCode::D))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[1];
            m_Speed.x                  = -0.1;
        }

        if (m_EngineContext->InputSys->GetKeyUp(KeyCode::D))
        {
            m_Animator.CurrentAnimator = m_Animator.Animators[0];
            m_Speed.x                  = 0;
        }

        m_EngineContext->PhysicsSys->Move(m_Controller, m_Speed, deltaTime);
        m_OwnerEntity->AddOrReplaceComponent<AnimatorComponent>(m_Animator);
    }

private:
    CharacterControllerComponent m_Controller;
    AnimatorComponent            m_Animator;
    glm::vec3                    m_Speed = {0, -9.8, 0};
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
        auto& thirdPersonFollowCam = camera.AddComponent<ThirdPersonFollowCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(CreateRef<EscScript>());

        auto normalMaterial = CreateRef<PhysicsMaterial>(0.4, 0.4, 0.4);

        // Create a terrain
        int    heightMapWidth                               = 1000;
        int    heightMapHeight                              = 1000;
        float  xScale                                       = 1;
        float  yScale                                       = 1;
        float  zScale                                       = 1;
        Entity terrain                                      = scene->CreateEntity("Terrain");
        terrain.GetComponent<TransformComponent>().Position = {
            -heightMapWidth * 0.5f * xScale, 0, -heightMapHeight * 0.5f * zScale}; // fix center
        auto& terrainComponent = terrain.AddComponent<TerrainComponent>();
        terrainComponent.TerrainHeightMap =
            Utils::GenerateSmoothTerrain(heightMapWidth, heightMapHeight); // create a 100 x 100 height map
        terrainComponent.XScale = xScale;
        terrainComponent.YScale = yScale;
        terrainComponent.ZScale = zScale;
        terrain.AddComponent<TerrainColliderComponent>(normalMaterial);
        auto& terrainRenderer = terrain.AddComponent<TerrainRendererComponent>();
        // terrainRenderer.BaseColor  = {0.6, 1, 0.6, 1}; // Light Green
        terrainRenderer.UseDiffuse             = true;
        terrainRenderer.DiffuseTextureFilePath = "Assets/Textures/dirt_01_diffuse.jpg";

        // Create a character
        Entity character                = scene->CreateEntity("Character");
        auto&  characterTransform       = character.GetComponent<TransformComponent>();
        characterTransform.Position.y   = 0.6;
        characterTransform.Scale        = {10, 10, 10};
        auto& characterMeshFilter       = character.AddComponent<MeshFilterComponent>();
        characterMeshFilter.FilePath    = "Assets/Models/Vampire/Vampire_Idle.dae";
        auto& characterMeshRenderer     = character.AddComponent<MeshRendererComponent>();
        characterMeshRenderer.BaseColor = {0.1, 0.1, 0.7, 1};
        auto& characterAnimator         = character.AddComponent<AnimatorComponent>();
        character.AddComponent<CharacterControllerComponent>();
        character.AddComponent<NativeScriptingComponent>(CreateRef<MyCharacterController>());

        // Load models and assign animations
        Model idle, walking, jumping;
        ModelLoader::LoadModel("Assets/Models/Vampire/Vampire_Idle.dae", idle);
        ModelLoader::LoadModel("Assets/Models/Vampire/Vampire_Walking.dae", walking);
        ModelLoader::LoadModel("Assets/Models/Vampire/Vampire_Jumping.dae", jumping);
        characterAnimator.Animators.emplace_back(CreateRef<Animator>(idle.Animations[0]));
        characterAnimator.Animators.emplace_back(CreateRef<Animator>(walking.Animations[0]));
        characterAnimator.Animators.emplace_back(CreateRef<Animator>(jumping.Animations[0]));
        characterAnimator.CurrentAnimator = characterAnimator.Animators[0];

        // set follow
        thirdPersonFollowCam.FollowEntity = character;
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