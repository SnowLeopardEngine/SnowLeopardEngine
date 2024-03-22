#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Random.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class SphereScript : public NativeScriptInstance
{
public:
    virtual void OnLoad() override
    {
        auto& transform    = m_OwnerEntity->GetComponent<TransformComponent>();
        auto& meshRenderer = m_OwnerEntity->GetComponent<MeshRendererComponent>();
        meshRenderer.Material->SetColor(
            "instanceColor",
            glm::vec4(Random::GetRandomFloat(), Random::GetRandomFloat(), Random::GetRandomFloat(), 1));
        meshRenderer.Material->SetVector("instancePosition", glm::vec4(transform.Position, 1));
        meshRenderer.Material->SetVector("instanceScale", glm::vec4(transform.Scale, 1));
    }
};

class EscScript : public NativeScriptInstance
{
public:
    virtual void OnColliderEnter() override
    {
        SNOW_LEOPARD_INFO("[SphereScript] OnColliderEnter");
        DesktopApp::GetInstance()->GetEngine()->GetContext()->AudioSys->Play("sounds/jump.mp3");
    }

    virtual void OnTick(float deltaTime) override
    {
        auto& inputSystem = DesktopApp::GetInstance()->GetEngine()->GetContext()->InputSys;

        if (inputSystem->GetKey(KeyCode::Escape))
        {
            DesktopApp::GetInstance()->Quit();
        }
    }
};

static Entity CreateSphere(const std::string& materialFilePath, const glm::vec3& position, const Ref<LogicScene>& scene)
{
    Entity sphere                       = scene->CreateEntity("Sphere");
    auto&  sphereTransform              = sphere.GetComponent<TransformComponent>();
    sphereTransform.Position            = position;
    sphereTransform.Scale               = {2, 2, 2};
    auto& sphereMeshFilter              = sphere.AddComponent<MeshFilterComponent>();
    sphereMeshFilter.UsePrimitive       = true;
    sphereMeshFilter.PrimitiveType      = MeshPrimitiveType::Sphere;
    auto& sphereMeshRenderer            = sphere.AddComponent<MeshRendererComponent>();
    sphereMeshRenderer.MaterialFilePath = materialFilePath;

    // Enable GPU Instancing
    sphereMeshRenderer.EnableInstancing = true;

    // Attach a script for changing instance color
    sphere.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(SphereScript));

    return sphere;
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
        cameraComponent.SkyboxMaterialFilePath             = "Assets/Materials/Skybox001.dzmaterial";

        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Load materials
        const std::string instancingMaterialPath = "Assets/Materials/InstancingTest.dzmaterial";

        // Create spheres to test materials
        for (size_t i = 0; i < 100; ++i)
        {
            glm::vec4 randomPosition(Random::GetRandomFloatRanged(-50, 50),
                                     Random::GetRandomFloatRanged(-50, 50),
                                     Random::GetRandomFloatRanged(-150, -50),
                                     1);
            CreateSphere(instancingMaterialPath, randomPosition, scene);
        }

        scene->SaveTo("Test.dzscene");
        scene->LoadFrom("Test.dzscene");
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
    REGISTER_TYPE(SphereScript);
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