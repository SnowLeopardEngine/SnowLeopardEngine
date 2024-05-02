#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Random.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
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

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_EngineContext = DesktopApp::GetInstance()->GetEngine()->GetContext();

        // Hide mouse cursor
        m_EngineContext->WindowSys->SetHideCursor(true);

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("RenderingSystem-Instancing", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox

        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Load materials
        const std::string instancingMaterialPath = "Assets/Materials/Next/Grass.dzmaterial";

        // Create grass quad instances
        for (size_t i = 0; i < 250; ++i)
        {
            glm::vec4 randomPosition(
                Random::GetRandomFloatRanged(-12, 12), 0, Random::GetRandomFloatRanged(-12, 12), 1);
            CreateQuad(instancingMaterialPath, randomPosition, scene);
        }

        // scene->SaveTo("Test.dzscene");
        // scene->LoadFrom("Test.dzscene");
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv) TRY
{
    REGISTER_TYPE(EscScript);

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - RenderingSystem-Instancing";
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