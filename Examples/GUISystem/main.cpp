#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Event/UIEvents.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Engine/EngineContext.h>
#include <SnowLeopardEngine/Function/IO/Resources.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

Model* g_Model;

class EscScript : public NativeScriptInstance
{
public:
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
        // m_EngineContext->WindowSys->SetHideCursor(true);

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("GUISystem", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox

        // camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform               = floor.GetComponent<TransformComponent>();
        floorTransform.Scale               = {50, 1, 50};
        auto& floorMeshFilter              = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType      = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer            = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.MaterialFilePath = "Assets/Materials/Next/White.dzmaterial";

        OzzModelLoadConfig config = {};
        config.OzzMeshPath        = "Assets/Models/Vampire/mesh.ozz";
        config.OzzSkeletonPath    = "Assets/Models/Vampire/skeleton.ozz";
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/animation_Dancing.ozz");
        bool ok = OzzModelLoader::Load(config, g_Model);

        // Create a character
        Entity character              = scene->CreateEntity("Character");
        auto&  characterTransform     = character.GetComponent<TransformComponent>();
        characterTransform.Position.y = 0.6;
        characterTransform.Scale      = {10, 10, 10};
        auto& characterMeshFilter     = character.AddComponent<MeshFilterComponent>();
        // characterMeshFilter.FilePath           = "Assets/Models/Walking.fbx";
        characterMeshFilter.Meshes             = &g_Model->Meshes;
        auto& characterMeshRenderer            = character.AddComponent<MeshRendererComponent>();
        characterMeshRenderer.MaterialFilePath = "Assets/Materials/Next/Vampire.dzmaterial";
        auto& animatorComponent                = character.AddComponent<AnimatorComponent>();

        Ref<AnimationClip> animation = g_Model->AnimationClips[0];
        Ref<Animator> animator = CreateRef<Animator>();
        Ref<AnimatorController> controller = CreateRef<AnimatorController>();
        controller->RegisterAnimationClip(animation);
        controller->SetEntryAnimationClip(animation);
        animatorComponent.Animator.SetController(controller);

        auto* tempRC = new RenderContext();

        const std::string uiImageMaterialPath = "Assets/Materials/Next/UIImage.dzmaterial";
        const std::string uiTextMaterialPath  = "Assets/Materials/Next/UIText.dzmaterial";

        // Create an background panel (image)
        Entity panelImage = scene->CreateEntity("PanelImage");
        auto&  panelRect  = panelImage.AddComponent<UI::RectTransformComponent>();

        panelRect.Size                  = {320, 240};
        panelRect.Pivot                 = {0.5, 0.5};
        panelRect.Pos                   = {512, 384, -0.1};
        auto& panelImageComp            = panelImage.AddComponent<UI::ImageComponent>();
        panelImageComp.TargetGraphic    = IO::Load("Assets/Textures/GUI/Panel/Window/Big.png", *tempRC);
        panelImageComp.MaterialFilePath = uiImageMaterialPath;

        // Create a play button
        Entity playButton     = scene->CreateEntity("PlayButton");
        auto&  playButtonRect = playButton.AddComponent<UI::RectTransformComponent>();
        playButtonRect.Size   = {100, 60};
        playButtonRect.Pivot  = {0.5, 0.5};
        playButtonRect.Pos    = {512, 384, 0};
        auto& playButtonComp  = playButton.AddComponent<UI::ButtonComponent>();
        playButtonComp.TintColor.TargetGraphic =
            IO::Load("Assets/Textures/GUI/Buttons/Rect-Medium/PlayText/Default.png", *tempRC);
        playButtonComp.MaterialFilePath = uiImageMaterialPath;

        Subscribe(m_ButtonClickedEventHandler);

        // Create a text
        Entity helloText     = scene->CreateEntity("HelloText");
        auto&  helloTextRect = helloText.AddComponent<UI::RectTransformComponent>();

        helloTextRect.Size             = {100, 30};
        helloTextRect.Pos              = {300, 100, 0};
        auto& helloTextComp            = helloText.AddComponent<UI::TextComponent>();
        helloTextComp.TextContent      = "Hello, GUI System!";
        helloTextComp.Color            = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
        helloTextComp.MaterialFilePath = uiTextMaterialPath;
    }

    virtual void OnUnload() { Unsubscribe(m_ButtonClickedEventHandler); }

private:
    EngineContext* m_EngineContext;

    EventHandler<UIButtonClickedEvent> m_ButtonClickedEventHandler = [this](const UIButtonClickedEvent& e) {
        auto   button       = e.GetButtonEntity();
        Entity buttonEntity = {button, m_EngineContext->SceneMngr->GetActiveScene().get()};
        SNOW_LEOPARD_INFO("Button (Name: {0}, UUID: {1}) was clicked!",
                          buttonEntity.GetName(),
                          to_string(buttonEntity.GetCoreUUID()));
    };
};

int main(int argc, char** argv)
{
    REGISTER_TYPE(EscScript);

    g_Model = new Model();

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - GUISystem";
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

    delete g_Model;

    return 0;
}