#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Function/Asset/TextureAsset.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
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
        cameraComponent.SkyboxMaterialFilePath             = "Assets/Materials/Skybox001.dzmaterial";

        // camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform               = floor.GetComponent<TransformComponent>();
        floorTransform.Scale               = {50, 1, 50};
        auto& floorMeshFilter              = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType      = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer            = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.MaterialFilePath = "Assets/Materials/Red.dzmaterial";

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
        characterMeshRenderer.MaterialFilePath = "Assets/Materials/Vampire.dzmaterial";
        auto& animatorComponent                = character.AddComponent<AnimatorComponent>();

        auto animator = CreateRef<Animator>(g_Model->AnimationClips[0]);
        animatorComponent.Controller.RegisterAnimator(animator);
        animatorComponent.Controller.SetEntryAnimator(animator);

        // Create a gui button
        Ref<Texture2DAsset> buttonTexture;
        Resources::Load<Texture2DAsset>("Assets/Textures/CoolGay.png", buttonTexture, false);
        Entity button                          = scene->CreateEntity("Button");
        auto&  buttonRect                      = button.AddComponent<UI::RectTransformComponent>();
        buttonRect.Size                        = {100, 60};
        buttonRect.Pivot                       = {0, 0};
        buttonRect.Pos                         = {10, 10, 0};
        auto& buttonComp                       = button.AddComponent<UI::ButtonComponent>();
        buttonComp.TintColor.TargetGraphicUUID = buttonTexture->GetUUID();
    }

private:
    EngineContext* m_EngineContext;
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