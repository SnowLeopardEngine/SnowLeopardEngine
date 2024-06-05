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
#include "SnowLeopardEngine/Function/Physics/OverlapInfo.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include <SnowLeopardEngine/Core/Base/Random.h>
#include <SnowLeopardEngine/Core/Time/Time.h>
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Engine/EngineContext.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

const std::string ImageMaterialPath = "Assets/Materials/Next/UIImage.dzmaterial";
const std::string TextMaterialPath  = "Assets/Materials/Next/UIText.dzmaterial";

using namespace SnowLeopardEngine;

Entity g_Cam;
bool   g_GameOver = false;

glm::vec3 GetTerrainHeightAtWorldPosition(const glm::vec3&        worldPosition,
                                          const TerrainComponent& terrainComponent,
                                          const glm::vec3&        terrainPos)
{
    glm::vec3 localPosition = worldPosition - terrainPos;

    int xIndex = static_cast<int>((localPosition.x / terrainComponent.XScale) +
                                  (terrainComponent.TerrainHeightMap.Data.size() / 2));
    int zIndex = static_cast<int>((localPosition.z / terrainComponent.ZScale) +
                                  (terrainComponent.TerrainHeightMap.Data[0].size() / 2));

    if (xIndex < 0 || xIndex >= terrainComponent.TerrainHeightMap.Data.size() || zIndex < 0 ||
        zIndex >= terrainComponent.TerrainHeightMap.Data[0].size())
    {
        return glm::vec3(localPosition.x, 0.0f, localPosition.z);
    }

    float terrainHeight = terrainComponent.TerrainHeightMap.Data[xIndex][zIndex];

    glm::vec3 worldPositionOnTerrain = localPosition;
    worldPositionOnTerrain.y         = terrainHeight * terrainComponent.YScale;

    return worldPositionOnTerrain;
}

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

        Entity playButton = g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithName("PlayButton");
        Entity playText   = g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithName("PlayText");

        // Raycast to test if on the ground.
        auto      origin = transform.Position - glm::vec3(0, controller.Height + 0.1f, 0); // under feet
        glm::vec3 direction(0, -1, 0);
        m_IsGrounded = g_EngineContext->PhysicsSys->SimpleRaycast(origin, direction, 0.15f);

        // Overlap to test if hit something
        bool        nearSpeaker = false;
        OverlapInfo overlapInfo = {};
        if (g_EngineContext->PhysicsSys->OverlapSphere(origin, 15.0f, overlapInfo))
        {
            for (auto& entity : overlapInfo.OverlappedEntities)
            {
                // SNOW_LEOPARD_INFO("[CharacterScript] Overlap with {0}", entity.GetName());

                if (entity.GetName() == "Speaker")
                {
                    nearSpeaker = true;
                }
            }
        }

        // Show UI
        if (nearSpeaker)
        {
            playButton.GetComponent<EntityStatusComponent>().IsEnabled = true;
            playText.GetComponent<EntityStatusComponent>().IsEnabled   = true;
        }
        else
        {
            playButton.GetComponent<EntityStatusComponent>().IsEnabled = false;
            playText.GetComponent<EntityStatusComponent>().IsEnabled   = false;
        }

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
            if (!m_PunchingTimerStart)
            {
                if (g_EngineContext->InputSys->GetKey(KeyCode::Q) && !g_GameOver)
                {
                    m_PunchingTimerStart = true;
                    m_PunchingAudioTimer = m_PunchingTime;
                    animator.CurrentAnimator.SetTrigger("Punching");
                    g_EngineContext->AudioSys->Play("DemoAssets/Audios/Punch.mp3");
                }
            }
            else
            {
                if (g_EngineContext->InputSys->GetKey(KeyCode::Q) && !g_GameOver)
                {
                    m_PunchingTimer = m_PunchingTime;
                    m_PunchingAudioTimer -= deltaTime;
                    if (m_PunchingAudioTimer <= 0)
                    {
                        g_EngineContext->AudioSys->Play("DemoAssets/Audios/Punch.mp3");
                        m_PunchingAudioTimer = m_PunchingTime;
                    }
                }

                m_PunchingTimer -= deltaTime;
                if (m_PunchingTimer <= 0)
                {
                    animator.CurrentAnimator.SetTrigger("PunchingEnd");
                    m_PunchingTimer      = m_PunchingTime;
                    m_PunchingTimerStart = false;
                }
            }

            if (m_IsGrounded && g_EngineContext->InputSys->GetKeyDown(KeyCode::Space) && !g_GameOver)
            {
                m_PreJump      = true;
                m_PrejumpTimer = m_PrejumpTime;
                animator.CurrentAnimator.SetTrigger("IsJump");
                m_IsJumping = true;
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

                g_EngineContext->AudioSys->Play("DemoAssets/Audios/Jumping.mp3");
            }
        }

        if (!m_PreJump && !m_PunchingTimerStart && !g_GameOver)
        {
            if (g_EngineContext->InputSys->GetKey(KeyCode::LeftShift))
            {
                factor = 1.5f;
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
        }

        if (m_IsGrounded && !m_LastFrameIsGrounded && !m_PreJump && m_IsJumping)
        {
            g_EngineContext->AudioSys->Play("DemoAssets/Audios/JumpLand.wav");
            m_IsJumping = false;
            animator.CurrentAnimator.SetTrigger("IsFallToGround");
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

        if (g_GameOver && !m_SetDeath)
        {
            animator.CurrentAnimator.SetTrigger("Death");
            m_SetDeath = true;
        }

        // Update
        m_VerticalSpeed += m_VerticalAcc * deltaTime;
        m_VerticalMovement   = m_VerticalSpeed * deltaTime * m_BaseFactor;
        m_HorizontalMovement = m_HorizontalVelocity * deltaTime * m_BaseFactor;
        m_HorizontalVelocity *= 0.8f; // damping

        // Set animator controller property
        animator.CurrentAnimator.SetFloat("HorizontalSpeed", horizontalInput);
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
    float     m_JumpSpeed        = 4;
    glm::vec2 m_HorizontalVelocity;
    glm::vec2 m_HorizontalMovement;

    bool  m_PreJump      = false;
    float m_PrejumpTime  = 0.7f;
    float m_PrejumpTimer = m_PrejumpTime;

    bool  m_PunchingTimerStart = false;
    float m_PunchingTime       = 1.0f;
    float m_PunchingTimer      = m_PunchingTime;

    float m_PunchingAudioTimer = m_PunchingTime;

    bool m_IsJumping           = false;
    bool m_IsGrounded          = false;
    bool m_LastFrameIsGrounded = false;

    bool m_SetDeath = false;
};

class EnemyScript : public NativeScriptInstance
{
public:
    virtual void OnLoad() override {}

    virtual void OnTick(float deltaTime) override
    {
        auto& transform  = m_OwnerEntity->GetComponent<TransformComponent>();
        auto& animator   = m_OwnerEntity->GetComponent<AnimatorComponent>();
        auto& controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>();

        Entity    player         = g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithName("Character");
        Entity    healthBarFront = g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithName("HealthBarFront");
        Entity    gameOverText   = g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithName("GameOverText");
        glm::vec3 movement(0.0f);

        if (player)
        {
            auto& playerTransform = player.GetComponent<TransformComponent>();
            auto  offset          = playerTransform.Position - transform.Position;

            float distance = glm::distance(playerTransform.Position, transform.Position);

            if (distance < 15.0f && !g_GameOver)
            {
                if (!m_PunchingTimerStart)
                {
                    if (healthBarFront)
                    {
                        auto& rect = healthBarFront.GetComponent<UI::RectTransformComponent>();
                        rect.Size.x -= 20;
                        if (rect.Size.x < 0)
                        {
                            // Player die
                            g_GameOver                                                   = true;
                            rect.Size.x                                                  = 0;
                            gameOverText.GetComponent<EntityStatusComponent>().IsEnabled = true;
                            g_EngineContext->AudioSys->Play("DemoAssets/Audios/Death.mp3");
                        }
                    }

                    m_PunchingAudioTimer = m_PunchingTime;
                    animator.CurrentAnimator.SetTrigger("Punching");
                    g_EngineContext->AudioSys->Play("DemoAssets/Audios/Punch.mp3");
                    player.GetComponent<AnimatorComponent>().CurrentAnimator.SetTrigger("RibHit");
                    m_PunchingTimerStart = true;
                }
            }
            else if (distance < 50.0f)
            {
                movement.x = offset.x;
                movement.z = offset.z;
                movement   = glm::normalize(movement) * 0.6f;
            }
            else if (distance < 200.0f)
            {
                movement.x = offset.x;
                movement.z = offset.z;
                movement   = glm::normalize(movement) * 1.1f;
            }
        }

        if (m_PunchingTimerStart)
        {
            m_PunchingTimer -= deltaTime;
            if (m_PunchingTimer <= 0)
            {
                player.GetComponent<AnimatorComponent>().CurrentAnimator.SetTrigger("RibHitEnd");
                animator.CurrentAnimator.SetTrigger("PunchingEnd");
                m_PunchingTimer      = m_PunchingTime;
                m_PunchingTimerStart = false;
            }
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
        m_HorizontalMovement = m_HorizontalVelocity * deltaTime * m_BaseFactor;
        m_HorizontalVelocity *= 0.8f; // damping

        // Set animator controller property
        animator.CurrentAnimator.SetFloat("HorizontalSpeed", horizontalInput);
    }

    void OnFixedTick() override
    {
        // Character controller moving
        auto& controller = m_OwnerEntity->GetComponent<CharacterControllerComponent>();
        g_EngineContext->PhysicsSys->Move(
            controller, glm::vec3(m_HorizontalMovement.x, -9.8, m_HorizontalMovement.y), Time::FixedDeltaTime);
    }

private:
    float     m_BaseFactor = 30.0f;
    glm::vec2 m_HorizontalVelocity;
    glm::vec2 m_HorizontalMovement;

    bool  m_PunchingTimerStart = false;
    float m_PunchingTime       = 1.0f;
    float m_PunchingTimer      = m_PunchingTime;

    float m_PunchingAudioTimer = m_PunchingTime;
};

class GameLoad final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_PlayerModel   = new Model();
        m_EnemyModel    = new Model();
        m_RenderContext = CreateScope<RenderContext>();
        LoadMainMenuScene();

        Subscribe(m_ButtonClickedEventHandler);
    }

    virtual void OnUnload() override final
    {
        Unsubscribe(m_ButtonClickedEventHandler);
        delete m_EnemyModel;
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
        camera.AddComponent<AudioListenerComponent>();
        auto& bgm       = camera.AddComponent<AudioSourceComponent>();
        bgm.AudioPath   = "DemoAssets/Audios/MainMenu.mp3";
        bgm.IsLoop      = true;
        bgm.IsSpatial   = false;
        bgm.PlayOnAwake = true;

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
        camera.AddComponent<AudioListenerComponent>();
        auto& bgm       = camera.AddComponent<AudioSourceComponent>();
        bgm.AudioPath   = "DemoAssets/Audios/OutDoor.mp3";
        bgm.IsLoop      = true;
        bgm.IsSpatial   = false;
        bgm.PlayOnAwake = true;
        bgm.Volume      = 0.1f;

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

        // Create a healthbar
        Entity healthBarBack                = m_GameScene->CreateEntity("HealthBarBack");
        auto&  healthBarBackRect            = healthBarBack.AddComponent<UI::RectTransformComponent>();
        healthBarBackRect.Size              = {200, 50};
        healthBarBackRect.Pivot             = {0.5, 0.5};
        healthBarBackRect.Pos               = {200, 680, -0.1};
        auto& healthBarBackImage            = healthBarBack.AddComponent<UI::ImageComponent>();
        healthBarBackImage.TargetGraphic    = IO::Load("DemoAssets/GUI/HealthBarBack.png", *m_RenderContext);
        healthBarBackImage.MaterialFilePath = ImageMaterialPath;

        Entity healthBarFront                = m_GameScene->CreateEntity("HealthBarFront");
        auto&  healthBarFrontRect            = healthBarFront.AddComponent<UI::RectTransformComponent>();
        healthBarFrontRect.Size              = {196, 46};
        healthBarFrontRect.Pivot             = {0, 0.5};
        healthBarFrontRect.Pos               = {102, 680, -0.05};
        auto& healthBarFrontImage            = healthBarFront.AddComponent<UI::ImageComponent>();
        healthBarFrontImage.TargetGraphic    = IO::Load("DemoAssets/GUI/HealthBarFront.png", *m_RenderContext);
        healthBarFrontImage.MaterialFilePath = ImageMaterialPath;

        // Create a GAME-OVER text
        Entity gameOverText               = m_GameScene->CreateEntity("GameOverText");
        auto&  gameOverTextRect           = gameOverText.AddComponent<UI::RectTransformComponent>();
        gameOverTextRect.Size             = {500, 200};
        gameOverTextRect.Pos              = {400, 600, 0};
        auto& gameOverTextComp            = gameOverText.AddComponent<UI::TextComponent>();
        gameOverTextComp.TextContent      = "GAME OVER";
        gameOverTextComp.FontSize         = 24;
        gameOverTextComp.Color            = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        gameOverTextComp.MaterialFilePath = TextMaterialPath;
        gameOverText.GetComponent<EntityStatusComponent>().IsEnabled = false;

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

        {
            // Create our player character
            OzzModelLoadConfig config = {};
            config.OzzMeshPath        = "Assets/Models/Vampire/mesh.ozz";
            config.OzzSkeletonPath    = "Assets/Models/Vampire/skeleton.ozz";
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Idle.ozz");        // 0
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Walking.ozz");     // 1
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Run.ozz");         // 2
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Jumping.ozz");     // 3
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Punching.ozz");    // 4
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Death.ozz");       // 5
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/ThrowObject.ozz"); // 6
            config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/RibHit.ozz");      // 7
            bool ok = OzzModelLoader::Load(config, m_PlayerModel);

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

            std::vector<Ref<AnimationClip>> animationClips;
            Ref<AnimatorController>         animatorController = CreateRef<AnimatorController>();
            for (const auto& clip : m_PlayerModel->AnimationClips)
            {
                animatorController->RegisterAnimationClip(clip);
                animationClips.emplace_back(clip);
            }
            animatorController->SetEntryAnimationClip(animationClips[0]); // Idle as default
            animatorComponent.CurrentAnimator.SetController(animatorController);
            animationClips[5]->IsLoop = false; // Death not loop

            // Parameters
            animatorController->RegisterParameters("HorizontalSpeed", 0.0f);
            animatorController->RegisterParameters("IsJump");         // Trigger
            animatorController->RegisterParameters("IsFallToGround"); // Trigger
            animatorController->RegisterParameters("Punching");       // Trigger
            animatorController->RegisterParameters("PunchingEnd");    // Trigger
            animatorController->RegisterParameters("Death");          // Trigger
            animatorController->RegisterParameters("RibHit");         // Trigger
            animatorController->RegisterParameters("RibHitEnd");      // Trigger

            // Transitions between Idle and Walking
            auto idle2Walk = animatorController->RegisterTransition(animationClips[0], animationClips[1], 0.5);
            idle2Walk->SetConditions("HorizontalSpeed", ConditionOperator::GreaterEqual, 1.0f);
            auto walk2Idle = animatorController->RegisterTransition(animationClips[1], animationClips[0], 0.5);
            walk2Idle->SetConditions("HorizontalSpeed", ConditionOperator::Less, 1.0f);

            // Transitions between Walking and Run
            auto walk2Run = animatorController->RegisterTransition(animationClips[1], animationClips[2], 0.5);
            walk2Run->SetConditions("HorizontalSpeed", ConditionOperator::GreaterEqual, 1.5f);
            auto run2Walk = animatorController->RegisterTransition(animationClips[2], animationClips[1], 0.5);
            run2Walk->SetConditions("HorizontalSpeed", ConditionOperator::Less, 1.5f);

            // Transitions between * and Jump
            auto idle2Jump = animatorController->RegisterTransition(animationClips[0], animationClips[3], 0.5);
            idle2Jump->AddTrigger("IsJump");
            auto jump2Idle = animatorController->RegisterTransition(animationClips[3], animationClips[0], 0.5);
            jump2Idle->AddTrigger("IsFallToGround");
            auto walk2Jump = animatorController->RegisterTransition(animationClips[1], animationClips[3], 0.5);
            walk2Jump->AddTrigger("IsJump");
            auto jump2Walk = animatorController->RegisterTransition(animationClips[3], animationClips[1], 0.5);
            jump2Walk->AddTrigger("IsFallToGround");
            auto run2Jump = animatorController->RegisterTransition(animationClips[2], animationClips[3], 0.5);
            run2Jump->AddTrigger("IsJump");
            auto jump2Run = animatorController->RegisterTransition(animationClips[3], animationClips[2], 0.5);
            jump2Run->AddTrigger("IsFallToGround");

            // Transitions between * and Punching
            auto idle2Punching = animatorController->RegisterTransition(animationClips[0], animationClips[4], 0.5);
            idle2Punching->AddTrigger("Punching");
            auto punching2Idle = animatorController->RegisterTransition(animationClips[4], animationClips[0], 0.5);
            punching2Idle->AddTrigger("PunchingEnd");
            auto walk2Punching = animatorController->RegisterTransition(animationClips[1], animationClips[4], 0.5);
            walk2Punching->AddTrigger("Punching");
            auto punching2Walk = animatorController->RegisterTransition(animationClips[4], animationClips[1], 0.5);
            punching2Walk->AddTrigger("PunchingEnd");
            auto run2Punching = animatorController->RegisterTransition(animationClips[2], animationClips[4], 0.5);
            run2Punching->AddTrigger("Punching");
            auto punching2Run = animatorController->RegisterTransition(animationClips[4], animationClips[2], 0.5);
            punching2Run->AddTrigger("PunchingEnd");

            // Transitions between * and Death
            auto idle2Death = animatorController->RegisterTransition(animationClips[0], animationClips[5], 0.5);
            idle2Death->AddTrigger("Death");
            auto walk2Death = animatorController->RegisterTransition(animationClips[1], animationClips[5], 0.5);
            walk2Death->AddTrigger("Death");
            auto run2Death = animatorController->RegisterTransition(animationClips[2], animationClips[5], 0.5);
            run2Death->AddTrigger("Death");
            auto jump2Death = animatorController->RegisterTransition(animationClips[3], animationClips[5], 0.5);
            jump2Death->AddTrigger("Death");
            auto punching2Death = animatorController->RegisterTransition(animationClips[4], animationClips[5], 0.5);
            punching2Death->AddTrigger("Death");
            auto ribHit2Death = animatorController->RegisterTransition(animationClips[7], animationClips[5], 0.5);
            ribHit2Death->AddTrigger("Death");

            // Transitions between * and RibHit
            auto idle2RibHit = animatorController->RegisterTransition(animationClips[0], animationClips[7], 0.5);
            idle2RibHit->AddTrigger("RibHit");
            auto ribHit2Idle = animatorController->RegisterTransition(animationClips[7], animationClips[0], 0.5);
            ribHit2Idle->AddTrigger("RibHitEnd");
            auto walk2RibHit = animatorController->RegisterTransition(animationClips[1], animationClips[7], 0.5);
            walk2RibHit->AddTrigger("RibHit");
            auto ribHit2Walk = animatorController->RegisterTransition(animationClips[7], animationClips[1], 0.5);
            ribHit2Walk->AddTrigger("RibHitEnd");
            auto run2RibHit = animatorController->RegisterTransition(animationClips[2], animationClips[7], 0.5);
            run2RibHit->AddTrigger("RibHit");
            auto ribHit2Run = animatorController->RegisterTransition(animationClips[7], animationClips[2], 0.5);
            ribHit2Run->AddTrigger("RibHitEnd");
            auto punching2RibHit = animatorController->RegisterTransition(animationClips[4], animationClips[7], 0.5);
            punching2RibHit->AddTrigger("RibHit");
            auto ribHit2Punching = animatorController->RegisterTransition(animationClips[7], animationClips[4], 0.5);
            ribHit2Punching->AddTrigger("RibHitEnd");

            auto& controller = character.AddComponent<CharacterControllerComponent>();
            character.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(CharacterScript));

            auto& tpCamControl        = camera.AddComponent<ThirdPersonFollowCameraControllerComponent>();
            tpCamControl.FollowEntity = character;
        }

        {
            // Create the enemy character
            OzzModelLoadConfig config = {};
            config.OzzMeshPath        = "DemoAssets/Models/Enemy/Enemy.ozzmesh";
            config.OzzSkeletonPath    = "DemoAssets/Models/Enemy/Enemy.ozzskeleton";
            config.OzzAnimationPaths.emplace_back("DemoAssets/Models/Enemy/EnemyIdle.ozz");
            config.OzzAnimationPaths.emplace_back("DemoAssets/Models/Enemy/EnemyWalking.ozz");
            config.OzzAnimationPaths.emplace_back("DemoAssets/Models/Enemy/EnemyRunning.ozz");
            config.OzzAnimationPaths.emplace_back("DemoAssets/Models/Enemy/EnemyPunching.ozz");
            config.OzzAnimationPaths.emplace_back("DemoAssets/Models/Enemy/EnemyDeath.ozz");
            bool ok = OzzModelLoader::Load(config, m_EnemyModel);

            Entity character                       = m_GameScene->CreateEntity("Enemy");
            auto&  characterTransform              = character.GetComponent<TransformComponent>();
            characterTransform.Position            = {-30, 50, -200};
            characterTransform.Scale               = {12, 12, 12};
            auto& characterMeshFilter              = character.AddComponent<MeshFilterComponent>();
            characterMeshFilter.Meshes             = &m_EnemyModel->Meshes;
            auto& characterMeshRenderer            = character.AddComponent<MeshRendererComponent>();
            characterMeshRenderer.MaterialFilePath = "DemoAssets/Models/Enemy/Enemy.dzmaterial";
            auto& animatorComponent                = character.AddComponent<AnimatorComponent>();

            std::vector<Ref<AnimationClip>> animationClips;
            Ref<AnimatorController>         animatorController = CreateRef<AnimatorController>();
            for (const auto& clip : m_EnemyModel->AnimationClips)
            {
                animatorController->RegisterAnimationClip(clip);
                animationClips.emplace_back(clip);
            }
            animatorController->SetEntryAnimationClip(animationClips[0]); // Idle as default
            animatorComponent.CurrentAnimator.SetController(animatorController);

            // Parameters
            animatorController->RegisterParameters("HorizontalSpeed", 0.0f);
            animatorController->RegisterParameters("Punching");    // Trigger
            animatorController->RegisterParameters("PunchingEnd"); // Trigger

            // Transitions between Idle and Walking
            auto idle2Walk = animatorController->RegisterTransition(animationClips[0], animationClips[1], 0.5);
            idle2Walk->SetConditions("HorizontalSpeed", ConditionOperator::GreaterEqual, 0.5f);
            auto walk2Idle = animatorController->RegisterTransition(animationClips[1], animationClips[0], 0.5);
            walk2Idle->SetConditions("HorizontalSpeed", ConditionOperator::Less, 0.5f);

            // Transitions between Walking and Run
            auto walk2Run = animatorController->RegisterTransition(animationClips[1], animationClips[2], 0.5);
            walk2Run->SetConditions("HorizontalSpeed", ConditionOperator::GreaterEqual, 1.0f);
            auto run2Walk = animatorController->RegisterTransition(animationClips[2], animationClips[1], 0.5);
            run2Walk->SetConditions("HorizontalSpeed", ConditionOperator::Less, 1.0f);

            // Transitions between * and Punching
            auto idle2Punching = animatorController->RegisterTransition(animationClips[0], animationClips[3], 0.5);
            idle2Punching->AddTrigger("Punching");
            auto punching2Idle = animatorController->RegisterTransition(animationClips[3], animationClips[0], 0.5);
            punching2Idle->AddTrigger("PunchingEnd");
            auto walk2Punching = animatorController->RegisterTransition(animationClips[1], animationClips[3], 0.5);
            walk2Punching->AddTrigger("Punching");
            auto punching2Walk = animatorController->RegisterTransition(animationClips[3], animationClips[1], 0.5);
            punching2Walk->AddTrigger("PunchingEnd");
            auto run2Punching = animatorController->RegisterTransition(animationClips[2], animationClips[3], 0.5);
            run2Punching->AddTrigger("Punching");
            auto punching2Run = animatorController->RegisterTransition(animationClips[3], animationClips[2], 0.5);
            punching2Run->AddTrigger("PunchingEnd");

            auto& controller = character.AddComponent<CharacterControllerComponent>();
            character.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EnemyScript));
        }

        // Create a speaker object to test 3D spatial sound
        Entity speaker            = m_GameScene->CreateEntity("Speaker");
        m_Speaker                 = speaker;
        auto& speakerTransform    = speaker.GetComponent<TransformComponent>();
        speakerTransform.Position = {-10, 30, 10};
        speakerTransform.Scale *= 5;
        auto& speakerMeshFilter              = speaker.AddComponent<MeshFilterComponent>();
        speakerMeshFilter.FilePath           = "DemoAssets/Models/Speaker/Speaker.dae";
        auto& speakerMeshRenderer            = speaker.AddComponent<MeshRendererComponent>();
        speakerMeshRenderer.MaterialFilePath = "DemoAssets/Models/Speaker/Speaker.dzmaterial";
        auto& speakderAudioSource            = speaker.AddComponent<AudioSourceComponent>();
        speakderAudioSource.AudioPath        = "DemoAssets/Audios/SpeakerBGM.mp3";
        speakderAudioSource.IsSpatial        = true;
        speakderAudioSource.IsLoop           = true;
        speakderAudioSource.PlayOnAwake      = false; // Use UI to turn on

        auto& speakerBoxCollider  = speaker.AddComponent<BoxColliderComponent>(normalMaterial);
        speakerBoxCollider.Size   = {5, 3, 3};
        auto& speakerBoxRigidbody = speaker.AddComponent<RigidBodyComponent>();

        // Create a play music button
        Entity playButton    = m_GameScene->CreateEntity("PlayButton");
        m_PlayMusicButtonID  = playButton.GetCoreUUID();
        auto& playButtonRect = playButton.AddComponent<UI::RectTransformComponent>();
        playButtonRect.Size  = {229, 44};
        playButtonRect.Pivot = {0.5, 0.5};
        playButtonRect.Pos   = {800, 360, 0};
        auto& playButtonComp = playButton.AddComponent<UI::ButtonComponent>();
        playButtonComp.TintColor.TargetGraphic =
            IO::Load("Assets/Textures/GUI/FantacyUI/Buttons/ButtonNormal.png", *m_RenderContext);
        playButtonComp.MaterialFilePath = ImageMaterialPath;
        // embedded text
        Entity playText               = m_GameScene->CreateEntity("PlayText");
        m_PlayMusicText               = playText;
        auto& playTextRect            = playText.AddComponent<UI::RectTransformComponent>();
        playTextRect.Size             = {229, 44};
        playTextRect.Pos              = {740, 350, 0};
        auto& playTextComp            = playText.AddComponent<UI::TextComponent>();
        playTextComp.TextContent      = "Play Music";
        playTextComp.FontSize         = 8;
        playTextComp.Color            = glm::vec4(1.0f, 0.8f, 0.0f, 1.0f);
        playTextComp.MaterialFilePath = TextMaterialPath;

        // Set disabled by default
        playButton.GetComponent<EntityStatusComponent>().IsEnabled = false;
        playText.GetComponent<EntityStatusComponent>().IsEnabled   = false;

        const std::string instancingMaterialPath = "Assets/Materials/Next/Grass.dzmaterial";

        // Create grass
        for (uint32_t i = 0; i < 250; ++i)
        {
            Entity quad          = m_GameScene->CreateEntity("GrassQuad");
            auto&  quadTransform = quad.GetComponent<TransformComponent>();
            quadTransform.Position =
                glm::vec3(Random::GetRandomFloatRanged(-200, 200), 0, Random::GetRandomFloatRanged(-200, 200));

            int terrainX = (quadTransform.Position.x + heightMapWidth * 0.5f * xScale) / xScale;
            int terrainZ = (quadTransform.Position.z + heightMapHeight * 0.5f * zScale) / zScale;

            quadTransform.Position.y = terrainComponent.TerrainHeightMap.Get(terrainZ, terrainX) * yScale + 5.0f;

            quadTransform.SetRotationEuler(glm::vec3(Random::GetRandomFloatRanged(-135, -90),
                                                     Random::GetRandomFloatRanged(0, 90),
                                                     Random::GetRandomFloatRanged(0, 10)));
            quadTransform.Scale               = {12, 24, 12};
            auto& quadMeshFilter              = quad.AddComponent<MeshFilterComponent>();
            quadMeshFilter.PrimitiveType      = MeshPrimitiveType::Quad;
            auto& quadMeshRenderer            = quad.AddComponent<MeshRendererComponent>();
            quadMeshRenderer.MaterialFilePath = instancingMaterialPath;
        }
    }

    void LoadMainMenuScene()
    {
        if (m_MainMenuScene == nullptr)
        {
            CreateMainMenuScene();
        }
        else
        {
            m_PlayMusicText.GetComponent<UI::TextComponent>().TextContent = "Play Music";
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

    void PlayOrStopMusic()
    {
        if (m_Speaker.GetComponent<AudioSourceComponent>().Clip->IsPlaying())
        {
            m_PlayMusicText.GetComponent<UI::TextComponent>().TextContent = "Play Music";
            m_Speaker.GetComponent<AudioSourceComponent>().Clip->Stop();
        }
        else
        {
            m_PlayMusicText.GetComponent<UI::TextComponent>().TextContent = "Stop Music";
            m_Speaker.GetComponent<AudioSourceComponent>().Clip->Play();
        }
    }

private:
    // Scenes
    Ref<LogicScene>      m_MainMenuScene = nullptr;
    Ref<LogicScene>      m_GameScene     = nullptr;
    Scope<RenderContext> m_RenderContext = nullptr;

    // Player
    Model* m_PlayerModel = nullptr;

    // Enemy
    Model* m_EnemyModel = nullptr;

    // Speaker
    Entity m_Speaker;
    Entity m_PlayMusicText;

    // Button UUIDs for event handling
    CoreUUID m_PlayButtonID;
    CoreUUID m_ExitButtonID;
    CoreUUID m_MenuButtonID;

    CoreUUID m_PlayMusicButtonID;

    EventHandler<UIButtonClickedEvent> m_ButtonClickedEventHandler = [this](const UIButtonClickedEvent& e) {
        auto   button       = e.GetButtonEntity();
        Entity buttonEntity = {button, g_EngineContext->SceneMngr->GetActiveScene().get()};
        SNOW_LEOPARD_INFO("Button (Name: {0}, UUID: {1}) was clicked!",
                          buttonEntity.GetName(),
                          to_string(buttonEntity.GetCoreUUID()));

        g_EngineContext->AudioSys->Play("DemoAssets/Audios/ButtonClick.wav");

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

        if (buttonID == m_PlayMusicButtonID)
        {
            PlayOrStopMusic();
        }
    };
};

int main(int argc, char** argv) TRY
{
    REGISTER_TYPE(CharacterScript);
    REGISTER_TYPE(EnemyScript);

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