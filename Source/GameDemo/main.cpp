#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Event/UIEvents.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include <SnowLeopardEngine/Core/Time/Time.h>
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Engine/EngineContext.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

const std::string ImageMaterialPath = "Assets/Materials/Next/UIImage.dzmaterial";
const std::string TextMaterialPath  = "Assets/Materials/Next/UIText.dzmaterial";

using namespace SnowLeopardEngine;

Entity g_Cam;

class CharacterScript : public NativeScriptInstance
{
public:
    virtual void OnLoad() override {}

    virtual void OnTick(float deltaTime) override
    {
        if (!g_Cam)
        {
            return;
        }

        m_LastFrameIsGrounded = m_IsGrounded;

        auto& transform  = m_OwnerEntity->GetComponent<TransformComponent>();
        auto& animator   = m_OwnerEntity->GetComponent<AnimatorComponent>();
        auto& controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>();

        // Raycast to test if on the ground.
        auto      origin = transform.Position - glm::vec3(0, controller.Height + 0.1f, 0); // under feet
        glm::vec3 direction(0, -1, 0);
        m_IsGrounded = g_EngineContext->PhysicsSys->SimpleRaycast(origin, direction, 0.15f);

        glm::vec3 forwardDirection(0, 0, -1);
        glm::vec3 rightDirection(-1, 0, 0);

        auto&     cameraTransform = g_Cam.GetComponent<TransformComponent>();
        glm::quat cameraRotation  = cameraTransform.GetRotation();

        glm::vec3 globalForward = cameraRotation * forwardDirection;
        glm::vec3 globalRight   = cameraRotation * rightDirection;

        float     factor = 1.0f;
        glm::vec3 movement(0.0f);

        if (!m_PreJump)
        {
            if (g_EngineContext->InputSys->GetKey(KeyCode::LeftShift))
            {
                factor = 2.0f;
            }

            if (g_EngineContext->InputSys->GetKey(KeyCode::W))
            {
                movement += globalForward * factor;
            }

            if (g_EngineContext->InputSys->GetKey(KeyCode::S))
            {
                movement -= globalForward * factor;
            }

            if (g_EngineContext->InputSys->GetKey(KeyCode::A))
            {
                movement += globalRight * factor;
            }

            if (g_EngineContext->InputSys->GetKey(KeyCode::D))
            {
                movement -= globalRight * factor;
            }

            if (m_IsGrounded && g_EngineContext->InputSys->GetKeyDown(KeyCode::Space))
            {
                m_PreJump      = true;
                m_PrejumpTimer = m_PrejumpTime;
                animator.Manager.m_Animators[0]->SetTrigger("IsJump");
                std::cout << "IsJump" << std::endl;
            }
        }
        else
        {
            m_PrejumpTimer -= deltaTime;
            if (m_PrejumpTimer <= 0)
            {
                m_IsGrounded    = false;
                m_VerticalSpeed = m_JumpSpeed;
                m_PrejumpTimer  = m_PrejumpTime;
                m_PreJump       = false;
            }
        }

        if (m_IsGrounded && !m_LastFrameIsGrounded && !m_PreJump)
        {
            animator.Manager.m_Animators[0]->SetTrigger("IsFallToGround");
            std::cout << "IsFallToGround" << std::endl;
        }

        float horizontalInput = glm::length(movement);

        if (horizontalInput > 0.0f)
        {
            m_HorizontalVelocity.x = movement.x;
            m_HorizontalVelocity.y = movement.z;

            // Apply rotation
            glm::vec3 targetDirection = glm::normalize(glm::vec3(movement.x, 0.0f, movement.z));
            glm::quat targetRotation  = glm::quatLookAtLH(targetDirection, glm::vec3(0, 1, 0));

            // SLERP
            float     lerpFactor      = 15.0f;
            auto      currentRotation = transform.GetRotation();
            glm::quat newRotation     = glm::slerp(currentRotation, targetRotation, lerpFactor * deltaTime);

            transform.SetRotation(newRotation);
        }

        // Update
        m_VerticalSpeed += m_VerticalAcc * deltaTime;
        m_VerticalMovement   = m_VerticalSpeed * deltaTime * m_BaseFactor;
        m_HorizontalMovement = m_HorizontalVelocity * deltaTime * m_BaseFactor;
        m_HorizontalVelocity *= 0.8f; // damping

        // Set animator controller property
        animator.Manager.m_Animators[0]->SetFloat("HorizontalSpeed", horizontalInput);
    }

    void OnFixedTick() override
    {
        // Character controller moving
        auto& controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>();
        g_EngineContext->PhysicsSys->Move(controller,
                                          glm::vec3(m_HorizontalMovement.x, m_VerticalMovement, m_HorizontalMovement.y),
                                          Time::FixedDeltaTime);
    }

private:
    float     m_BaseFactor       = 30.0f;
    float     m_VerticalAcc      = -9.8f;
    float     m_VerticalSpeed    = 0;
    float     m_VerticalMovement = 0;
    float     m_JumpSpeed        = 5;
    glm::vec2 m_HorizontalVelocity;
    glm::vec2 m_HorizontalMovement;

    bool  m_PreJump      = false;
    float m_PrejumpTime  = 0.7f;
    float m_PrejumpTimer = 0.7f;

    bool m_IsGrounded          = false;
    bool m_LastFrameIsGrounded = false;
};

class GameLoad final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_PlayerModel   = new Model();
        m_RenderContext = CreateScope<RenderContext>();
        LoadMainMenuScene();
        PlayMainMenuBGM();

        Subscribe(m_ButtonClickedEventHandler);
    }

    virtual void OnUnload() override final
    {
        Unsubscribe(m_ButtonClickedEventHandler);
        delete m_PlayerModel;
    }

private:
    void CreateMainMenuScene()
    {
        // Create a Main Menu scene and set active
        m_MainMenuScene = g_EngineContext->SceneMngr->CreateScene("MainMenu", false);

        // Create a camera
        Entity camera              = m_MainMenuScene->CreateEntity("MainCamera");
        g_Cam                      = camera;
        auto& cameraTransform      = camera.GetComponent<TransformComponent>();
        cameraTransform.Position   = {0, 10, 30};
        auto& cameraComponent      = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags = CameraClearFlags::Skybox; // Enable skybox

        // Create a background panel image
        Entity bgImage               = m_MainMenuScene->CreateEntity("Bg");
        auto&  bgRect                = bgImage.AddComponent<UI::RectTransformComponent>();
        bgRect.Size                  = {1280, 720};
        bgRect.Pivot                 = {0.5, 0.5};
        bgRect.Pos                   = {640, 360, -0.1};
        auto& bgImageComp            = bgImage.AddComponent<UI::ImageComponent>();
        bgImageComp.TargetGraphic    = IO::Load("Assets/Textures/GUI/Panel/Window/Big.png", *m_RenderContext);
        bgImageComp.MaterialFilePath = ImageMaterialPath;

        // Create a game name text
        Entity gameNameText               = m_MainMenuScene->CreateEntity("GameNameText");
        auto&  gameNameTextRect           = gameNameText.AddComponent<UI::RectTransformComponent>();
        gameNameTextRect.Pos              = {400, 500, 0};
        auto& gameNameTextComp            = gameNameText.AddComponent<UI::TextComponent>();
        gameNameTextComp.TextContent      = "Untitled 3D RPG Game DEMO";
        gameNameTextComp.Color            = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f);
        gameNameTextComp.MaterialFilePath = TextMaterialPath;

        // Create a play button
        Entity playButton    = m_MainMenuScene->CreateEntity("PlayButton");
        m_PlayButtonID       = playButton.GetCoreUUID();
        auto& playButtonRect = playButton.AddComponent<UI::RectTransformComponent>();
        playButtonRect.Size  = {229, 44};
        playButtonRect.Pivot = {0.5, 0.5};
        playButtonRect.Pos   = {640, 360, 0};
        auto& playButtonComp = playButton.AddComponent<UI::ButtonComponent>();
        playButtonComp.TintColor.TargetGraphic =
            IO::Load("Assets/Textures/GUI/FantacyUI/Buttons/ButtonNormal.png", *m_RenderContext);
        playButtonComp.MaterialFilePath = ImageMaterialPath;
        // embedded text
        Entity playText               = m_MainMenuScene->CreateEntity("PlayText");
        auto&  playTextRect           = playText.AddComponent<UI::RectTransformComponent>();
        playTextRect.Pos              = {610, 350, 0};
        auto& playTextComp            = playText.AddComponent<UI::TextComponent>();
        playTextComp.TextContent      = "Play";
        playTextComp.FontSize         = 8;
        playTextComp.Color            = glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);
        playTextComp.MaterialFilePath = TextMaterialPath;

        // Create an exit button
        Entity exitButton    = m_MainMenuScene->CreateEntity("ExitButton");
        m_ExitButtonID       = exitButton.GetCoreUUID();
        auto& exitButtonRect = exitButton.AddComponent<UI::RectTransformComponent>();
        exitButtonRect.Size  = {229, 44};
        exitButtonRect.Pivot = {0.5, 0.5};
        exitButtonRect.Pos   = {640, 290, 0};
        auto& exitButtonComp = exitButton.AddComponent<UI::ButtonComponent>();
        exitButtonComp.TintColor.TargetGraphic =
            IO::Load("Assets/Textures/GUI/FantacyUI/Buttons/ButtonNormal.png", *m_RenderContext);
        exitButtonComp.MaterialFilePath = ImageMaterialPath;
        // embedded text
        Entity exitText               = m_MainMenuScene->CreateEntity("ExitText");
        auto&  exitTextRect           = exitText.AddComponent<UI::RectTransformComponent>();
        exitTextRect.Pos              = {610, 280, 0};
        auto& exitTextComp            = exitText.AddComponent<UI::TextComponent>();
        exitTextComp.TextContent      = "Exit";
        exitTextComp.FontSize         = 8;
        exitTextComp.Color            = glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);
        exitTextComp.MaterialFilePath = TextMaterialPath;
    }

    void CreateGameScene()
    {
        // Create a Game scene
        m_GameScene = g_EngineContext->SceneMngr->CreateScene("Game");

        // Create a camera
        Entity camera                                      = m_GameScene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox

        // Create a menu button
        Entity menuButton    = m_GameScene->CreateEntity("MenuButton");
        m_MenuButtonID       = menuButton.GetCoreUUID();
        auto& menuButtonRect = menuButton.AddComponent<UI::RectTransformComponent>();
        menuButtonRect.Size  = {40, 40};
        menuButtonRect.Pivot = {0.5, 0.5};
        menuButtonRect.Pos   = {50, 680, 0};
        auto& menuButtonComp = menuButton.AddComponent<UI::ButtonComponent>();
        menuButtonComp.TintColor.TargetGraphic =
            IO::Load("Assets/Textures/GUI/FantacyUI/Buttons/Menu.png", *m_RenderContext);
        menuButtonComp.MaterialFilePath = ImageMaterialPath;

        // Create a terrain
        auto     normalMaterial = CreateRef<PhysicsMaterial>(0.4, 0.4, 0.4);
        uint32_t xSize = 1000, ySize = 1000;
        int      heightMapWidth                             = xSize;
        int      heightMapHeight                            = ySize;
        float    xScale                                     = 10;
        float    yScale                                     = 1;
        float    zScale                                     = 10;
        Entity   terrain                                    = m_GameScene->CreateEntity("Terrain");
        terrain.GetComponent<TransformComponent>().Position = {
            -heightMapWidth * 0.5f * xScale, 0, -heightMapHeight * 0.5f * zScale}; // fix center
        auto& terrainComponent            = terrain.AddComponent<TerrainComponent>();
        terrainComponent.TerrainHeightMap = Utils::GenerateRandomHeightMap(xSize, ySize, 50);
        terrainComponent.XScale           = xScale;
        terrainComponent.YScale           = yScale;
        terrainComponent.ZScale           = zScale;
        terrain.AddComponent<TerrainColliderComponent>(normalMaterial);
        auto& terrainRenderer            = terrain.AddComponent<TerrainRendererComponent>();
        terrainRenderer.MaterialFilePath = "Assets/Materials/Next/DefaultTerrain.dzmaterial";

        // Create our player character
        OzzModelLoadConfig config = {};
        config.OzzMeshPath        = "Assets/Models/Vampire/mesh.ozz";
        config.OzzSkeletonPath    = "Assets/Models/Vampire/skeleton.ozz";
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Idle.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Walking.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Run.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Jumping.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Punching.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Death.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/ThrowObject.ozz");
        bool ok = OzzModelLoader::Load(config, m_PlayerModel);

        // Create a character
        Entity character              = m_GameScene->CreateEntity("Character");
        auto&  characterTransform     = character.GetComponent<TransformComponent>();
        characterTransform.Position.y = 50;
        characterTransform.Scale      = {10, 10, 10};
        characterTransform.SetRotationEuler(glm::vec3(0, 180, 0));
        auto& characterMeshFilter              = character.AddComponent<MeshFilterComponent>();
        characterMeshFilter.Meshes             = &m_PlayerModel->Meshes;
        auto& characterMeshRenderer            = character.AddComponent<MeshRendererComponent>();
        characterMeshRenderer.MaterialFilePath = "Assets/Materials/Next/Vampire.dzmaterial";
        auto& animatorComponent                = character.AddComponent<AnimatorComponent>();

        Ref<Animator> animator = CreateRef<Animator>();
        std::vector<Ref<AnimationClip>> animationClips;
        Ref<AnimatorController> animatorController = CreateRef<AnimatorController>();
        for (const auto& clip : m_PlayerModel->AnimationClips)
        {
            animatorController->RegisterAnimationClip(clip);
            animationClips.emplace_back(clip);
        }
        animatorComponent.Manager.RegisterAnimator(animator);
        animatorController->SetEntryAnimationClip(animationClips[0]); // Idle as default
        animator->SetController(animatorController);

        // Parameters
        animatorController->RegisterParameters("HorizontalSpeed", 0.0f);
        animatorController->RegisterParameters("IsJump");         // Trigger
        animatorController->RegisterParameters("IsFallToGround"); // Trigger

        // Transitions between Idle and Walking
        auto idle2Walk = animatorController->RegisterTransition(animationClips[0], animationClips[1], 0);
        idle2Walk->SetConditions("HorizontalSpeed", ConditionOperator::GreaterEqual, 1.0f);
        auto walk2Idle = animatorController->RegisterTransition(animationClips[1], animationClips[0], 0);
        walk2Idle->SetConditions("HorizontalSpeed", ConditionOperator::Less, 1.0f);

        // Transitions between Walking and Run
        auto walk2Run = animatorController->RegisterTransition(animationClips[1], animationClips[2], 0);
        walk2Run->SetConditions("HorizontalSpeed", ConditionOperator::GreaterEqual, 2.0f);
        auto run2Walk = animatorController->RegisterTransition(animationClips[2], animationClips[1], 0);
        run2Walk->SetConditions("HorizontalSpeed", ConditionOperator::Less, 2.0f);

        // Transitions between * and Jump
        auto idle2Jump = animatorController->RegisterTransition(animationClips[0], animationClips[3], 0);
        idle2Jump->SetConditions("IsJump");
        auto jump2Idle = animatorController->RegisterTransition(animationClips[3], animationClips[0], 0);
        jump2Idle->SetConditions("IsFallToGround");
        auto walk2Jump = animatorController->RegisterTransition(animationClips[1], animationClips[3], 0);
        walk2Jump->SetConditions("IsJump");
        auto jump2Walk = animatorController->RegisterTransition(animationClips[3], animationClips[1], 0);
        jump2Walk->SetConditions("IsFallToGround");
        auto run2Jump = animatorController->RegisterTransition(animationClips[2], animationClips[3], 0);
        run2Jump->SetConditions("IsJump");
        auto jump2Run = animatorController->RegisterTransition(animationClips[3], animationClips[2], 0);
        jump2Run->SetConditions("IsFallToGround");

        auto& controller = character.AddComponent<CharacterControllerComponent>();
        character.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(CharacterScript));

        auto& tpCamControl        = camera.AddComponent<ThirdPersonFollowCameraControllerComponent>();
        tpCamControl.FollowEntity = character;
    }

    void LoadMainMenuScene()
    {
        if (m_MainMenuScene == nullptr)
        {
            CreateMainMenuScene();
        }
        g_EngineContext->SceneMngr->SetActiveScene(m_MainMenuScene);
    }

    void LoadGameScene()
    {
        if (m_GameScene == nullptr)
        {
            CreateGameScene();
        }
        g_EngineContext->SceneMngr->SetActiveScene(m_GameScene);
    }

    void PlayMainMenuBGM() { g_EngineContext->AudioSys->Play("assets/audios/MainMenu.mp3"); }

private:
    // Scenes
    Ref<LogicScene>      m_MainMenuScene = nullptr;
    Ref<LogicScene>      m_GameScene     = nullptr;
    Scope<RenderContext> m_RenderContext = nullptr;

    // Player
    Model* m_PlayerModel = nullptr;

    // Button UUIDs for event handling
    CoreUUID m_PlayButtonID;
    CoreUUID m_ExitButtonID;
    CoreUUID m_MenuButtonID;

    EventHandler<UIButtonClickedEvent> m_ButtonClickedEventHandler = [this](const UIButtonClickedEvent& e) {
        auto   button       = e.GetButtonEntity();
        Entity buttonEntity = {button, g_EngineContext->SceneMngr->GetActiveScene().get()};
        SNOW_LEOPARD_INFO("Button (Name: {0}, UUID: {1}) was clicked!",
                          buttonEntity.GetName(),
                          to_string(buttonEntity.GetCoreUUID()));

        g_EngineContext->AudioSys->Play("assets/audios/ButtonClick.wav");

        auto buttonID = buttonEntity.GetCoreUUID();

        if (buttonID == m_PlayButtonID)
        {
            // Load game scene
            LoadGameScene();
        }

        if (buttonID == m_ExitButtonID)
        {
            // Exit game
            DesktopApp::GetInstance()->Quit();
        }

        if (buttonID == m_MenuButtonID)
        {
            // Show Menu
            LoadMainMenuScene();
        }
    };
};

int main(int argc, char** argv) TRY
{
    REGISTER_TYPE(CharacterScript);

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title  = "Final Game DEMO";
    initInfo.Engine.Window.Width  = 1280;
    initInfo.Engine.Window.Height = 720;
    DesktopApp app(argc, argv);

    if (!app.Init(initInfo))
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    // To load the game
    app.GetEngine()->AddLifeTimeComponent(CreateRef<GameLoad>());

    if (!app.PostInit())
    {
        std::cerr << "Failed to post initialize the application!" << std::endl;
        return 1;
    }

    app.Run();

    return 0;
}
CATCH