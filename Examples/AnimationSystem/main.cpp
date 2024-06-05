#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
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
        m_EngineContext->WindowSys->SetHideCursor(true);

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("AnimationSystem", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox

        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform               = floor.GetComponent<TransformComponent>();
        floorTransform.Scale               = {50, 1, 50};
        auto& floorMeshFilter              = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType      = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer            = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.MaterialFilePath = "Assets/Materials/Next/Red.dzmaterial";

        OzzModelLoadConfig config = {};
        config.OzzMeshPath        = "Assets/Models/Vampire/mesh.ozz";
        config.OzzSkeletonPath    = "Assets/Models/Vampire/skeleton.ozz";
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/animation_Dancing.ozz");
        config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/Walking.ozz");
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

        Ref<AnimationClip> danceAnimation = g_Model->AnimationClips[0];
        Ref<AnimationClip> walkingAnimation = g_Model->AnimationClips[1];

        Ref<Animator> animator = CreateRef<Animator>();
        Ref<AnimatorController> controller = CreateRef<AnimatorController>();

        controller->RegisterAnimationClip(danceAnimation);
        controller->RegisterAnimationClip(walkingAnimation);
        controller->SetEntryAnimationClip(danceAnimation);

        animatorComponent.CurrentAnimator.SetController(controller);
        controller->RegisterParameters("Walking");
        auto idle2Walk = controller->RegisterTransition(danceAnimation, walkingAnimation, 1);
        idle2Walk->SetConditions("Walking");
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv) TRY
{
    REGISTER_TYPE(EscScript);

    g_Model = new Model();

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

    delete g_Model;

    return 0;
}
CATCH