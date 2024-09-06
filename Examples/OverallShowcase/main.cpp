#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Random.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/IO/OzzModelLoader.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/IO/Resources.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

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

static Entity CreateQuad(const std::string& materialFilePath, const glm::vec3& position, const Ref<LogicScene>& scene)
{
    Entity quad            = scene->CreateEntity("GrassQuad");
    auto&  quadTransform   = quad.GetComponent<TransformComponent>();
    quadTransform.Position = position;
    quadTransform.SetRotationEuler(glm::vec3(Random::GetRandomFloatRanged(-135, -90),
                                             Random::GetRandomFloatRanged(0, 90),
                                             Random::GetRandomFloatRanged(0, 10)));
    quadTransform.Scale               = {4, 8, 4};
    auto& quadMeshFilter              = quad.AddComponent<MeshFilterComponent>();
    quadMeshFilter.PrimitiveType      = MeshPrimitiveType::Quad;
    auto& quadMeshRenderer            = quad.AddComponent<MeshRendererComponent>();
    quadMeshRenderer.MaterialFilePath = materialFilePath;

    // Enable GPU Instancing
    quadMeshRenderer.EnableInstancing = true;

    return quad;
}

static Entity CreateCharacter(const Ref<LogicScene>& scene, Model* model, const glm::vec3& position)
{
    OzzModelLoadConfig config = {};
    config.OzzMeshPath        = "Assets/Models/Vampire/mesh.ozz";
    config.OzzSkeletonPath    = "Assets/Models/Vampire/skeleton.ozz";
    config.OzzAnimationPaths.emplace_back("Assets/Models/Vampire/animation_Dancing.ozz");
    bool ok = OzzModelLoader::Load(config, model);

    // Create a character
    Entity character            = scene->CreateEntity("Character");
    auto&  characterTransform   = character.GetComponent<TransformComponent>();
    characterTransform.Position = position;
    characterTransform.Scale    = {5, 5, 5};
    auto& characterMeshFilter   = character.AddComponent<MeshFilterComponent>();
    // characterMeshFilter.FilePath           = "Assets/Models/Walking.fbx";
    characterMeshFilter.Meshes             = &model->Meshes;
    auto& characterMeshRenderer            = character.AddComponent<MeshRendererComponent>();
    characterMeshRenderer.MaterialFilePath = "Assets/Materials/Next/Vampire.dzmaterial";
    auto& animatorComponent                = character.AddComponent<AnimatorComponent>();
    auto  controller                       = CreateRef<AnimatorController>();
    controller->RegisterAnimationClip(model->AnimationClips[0]);
    controller->SetEntryAnimationClip(model->AnimationClips[0]);
    animatorComponent.CurrentAnimator.SetController(controller);

    return character;
}

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_EngineContext = DesktopApp::GetInstance()->GetEngine()->GetContext();

        // Hide mouse cursor
        m_EngineContext->WindowSys->SetHideCursor(true);

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("OverallShowcase", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox

        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Load materials
        const std::string instancingMaterialPath = "Assets/Materials/Next/Grass.dzmaterial";

        // // Create grass quad instances
        // for (size_t i = 0; i < 10000; ++i)
        // {
        //     glm::vec4 randomPosition(
        //         Random::GetRandomFloatRanged(-60, 60), 0, Random::GetRandomFloatRanged(-60, 60), 1);
        //     CreateQuad(instancingMaterialPath, randomPosition, scene);
        // }

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform               = floor.GetComponent<TransformComponent>();
        floorTransform.Scale               = {150, 1, 150};
        auto& floorMeshFilter              = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType      = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer            = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.MaterialFilePath = "Assets/Materials/Next/Red.dzmaterial";

        for (int x = -60; x < 60; x += 30)
        {
            for (int y = -60; y < 60; y += 30)
            {
                Model* model = new Model();
                m_Models.emplace_back(model);
                CreateCharacter(scene, model, {x, 0, y});
            }
        }

        // scene->SaveTo("Test.dzscene");
        // scene->LoadFrom("Test.dzscene");
    }

    void OnUnload() override
    {
        for (Model* model : m_Models)
        {
            delete model;
            model = nullptr;
        }
    }

private:
    std::vector<Model*> m_Models;
    EngineContext*      m_EngineContext;
};

int main(int argc, char** argv) TRY
{
    REGISTER_TYPE(EscScript);

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title      = "Example - OverallShowcase";
    initInfo.Engine.Window.Fullscreen = true;
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
CATCH