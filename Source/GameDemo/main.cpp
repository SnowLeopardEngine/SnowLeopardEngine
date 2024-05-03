#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Event/UIEvents.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
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

class CharacterScript : public NativeScriptInstance
{
public:
    virtual void OnLoad() override
    {
        m_Controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>();
        m_Animator   = m_OwnerEntity->GetComponent<AnimatorComponent>();
    }

    virtual void OnTick(float deltaTime) override
    {
        if (g_EngineContext->InputSys->GetKey(KeyCode::W))
        {
            m_Speed.z = 0.1;
        }

        if (g_EngineContext->InputSys->GetKeyUp(KeyCode::W))
        {
            m_Speed.z = 0.0;
        }

        if (g_EngineContext->InputSys->GetKey(KeyCode::S))
        {
            m_Speed.z = -0.1;
        }

        if (g_EngineContext->InputSys->GetKeyUp(KeyCode::S))
        {
            m_Speed.z = 0;
        }

        if (g_EngineContext->InputSys->GetKey(KeyCode::A))
        {
            m_Speed.x = 0.1;
        }

        if (g_EngineContext->InputSys->GetKeyUp(KeyCode::A))
        {
            m_Speed.x = 0;
        }

        if (g_EngineContext->InputSys->GetKey(KeyCode::D))
        {
            m_Speed.x = -0.1;
        }

        if (g_EngineContext->InputSys->GetKeyUp(KeyCode::D))
        {
            m_Speed.x = 0;
        }

        g_EngineContext->PhysicsSys->Move(m_Controller, m_Speed, deltaTime);
        m_OwnerEntity->AddOrReplaceComponent<AnimatorComponent>(m_Animator);
    }

private:
    CharacterControllerComponent m_Controller;
    AnimatorComponent            m_Animator;
    glm::vec3                    m_Speed = {0, -9.8, 0};
};

class GameLoad final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_PlayerModel   = new Model();
        m_RenderContext = CreateScope<RenderContext>();
        CreateMainMenuScene();
        CreateGameScene();
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
        Entity camera                                      = m_MainMenuScene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox

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
        float    xScale                                     = 1;
        float    yScale                                     = 1;
        float    zScale                                     = 1;
        Entity   terrain                                    = m_GameScene->CreateEntity("Terrain");
        terrain.GetComponent<TransformComponent>().Position = {
            -heightMapWidth * 0.5f * xScale, 0, -heightMapHeight * 0.5f * zScale}; // fix center
        auto& terrainComponent            = terrain.AddComponent<TerrainComponent>();
        terrainComponent.TerrainHeightMap = Utils::GenerateRandomHeightMap(xSize, ySize);
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
        Entity character                       = m_GameScene->CreateEntity("Character");
        auto&  characterTransform              = character.GetComponent<TransformComponent>();
        characterTransform.Position.y          = 10;
        characterTransform.Scale               = {10, 10, 10};
        auto& characterMeshFilter              = character.AddComponent<MeshFilterComponent>();
        characterMeshFilter.Meshes             = &m_PlayerModel->Meshes;
        auto& characterMeshRenderer            = character.AddComponent<MeshRendererComponent>();
        characterMeshRenderer.MaterialFilePath = "Assets/Materials/Next/Vampire.dzmaterial";
        auto& animatorComponent                = character.AddComponent<AnimatorComponent>();
        auto  animator                         = CreateRef<Animator>(m_PlayerModel->AnimationClips[0]);
        animatorComponent.Controller.RegisterAnimator(animator);
        animatorComponent.Controller.SetEntryAnimator(animator);
        auto& controller = character.AddComponent<CharacterControllerComponent>();
        character.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(CharacterScript));

        auto& tpCamControl        = camera.AddComponent<ThirdPersonFollowCameraControllerComponent>();
        tpCamControl.FollowEntity = character;
    }

    void LoadMainMenuScene() { g_EngineContext->SceneMngr->SetActiveScene(m_MainMenuScene); }

    void LoadGameScene() { g_EngineContext->SceneMngr->SetActiveScene(m_GameScene); }

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