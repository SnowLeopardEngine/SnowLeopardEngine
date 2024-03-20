#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Random.h"
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
        auto      quaternion = transform.GetRotation();
        glm::vec4 vec4Representation(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
        meshRenderer.Material->SetVector("instanceQuaternion", vec4Representation);
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
    Entity sphere            = scene->CreateEntity("Sphere");
    auto&  sphereTransform   = sphere.GetComponent<TransformComponent>();
    sphereTransform.Position = position;
    sphereTransform.SetRotationEuler(glm::vec3(-90, 0, 0));
    sphereTransform.Scale               = {4, 8, 4};
    auto& sphereMeshFilter              = sphere.AddComponent<MeshFilterComponent>();
    sphereMeshFilter.PrimitiveType      = MeshPrimitiveType::Quad;
    auto& sphereMeshRenderer            = sphere.AddComponent<MeshRendererComponent>();
    sphereMeshRenderer.MaterialFilePath = materialFilePath;

    // Enable GPU Instancing
    sphereMeshRenderer.EnableInstancing = true;

    // Attach a script for changing instance color
    sphere.AddComponent<NativeScriptingComponent>(CreateRef<SphereScript>());

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
        cameraComponent.SkyboxMaterial = DzMaterial::LoadFromPath("Assets/Materials/Skybox001.dzmaterial");

        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(CreateRef<EscScript>());

        // Load materials
        const std::string instancingMaterialPath = "Assets/Materials/InstancingTest.dzmaterial";

        // Create spheres to test materials
        for (size_t i = 0; i < 250; ++i)
        {
            glm::vec4 randomPosition(
                Random::GetRandomFloatRanged(-25, 25), 0, Random::GetRandomFloatRanged(-60, -10), 1);
            CreateSphere(instancingMaterialPath, randomPosition, scene);
        }
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
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