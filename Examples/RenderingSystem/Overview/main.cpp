#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>
#include <SnowLeopardEngine/Core/Base/Random.h>

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

static Entity CreateSphere(const std::string& materialFilePath, const glm::vec3& position, const Ref<LogicScene>& scene)
{
    Entity sphere                       = scene->CreateEntity("Sphere");
    auto&  sphereTransform              = sphere.GetComponent<TransformComponent>();
    sphereTransform.Position            = position;
    sphereTransform.Scale               = {2, 2, 2};
    auto& sphereMeshFilter              = sphere.AddComponent<MeshFilterComponent>();
    sphereMeshFilter.PrimitiveType      = MeshPrimitiveType::Sphere;
    auto& sphereMeshRenderer            = sphere.AddComponent<MeshRendererComponent>();
    sphereMeshRenderer.MaterialFilePath = materialFilePath;

    return sphere;
}

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

static Entity CreatePointLight(const glm::vec3& position, const glm::vec3& color, const Ref<LogicScene>& scene)
{
    Entity pointLight            = scene->CreateEntity("Point Light");
    auto&  pointLightTransform   = pointLight.GetComponent<TransformComponent>();
    pointLightTransform.Position = position;
    auto& pointLightComponent    = pointLight.AddComponent<PointLightComponent>();
    pointLightComponent.Color    = color;

    pointLightComponent.Intensity = 100; // To test HDR & ToneMapping

    return pointLight;
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
        auto scene = m_EngineContext->SceneMngr->CreateScene("RenderingSystem-Overview", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox

        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Load materials
        const std::string deferredWhiteMaterialFilePath = "Assets/Materials/Next/White.dzmaterial";
        const std::string deferredPBRMaterialFilePath   = "Assets/Materials/Next/RustedIronPBRDeferred.dzmaterial";
        const std::string pureMetalMaterialFilePath     = "Assets/Materials/Next/PureMetal.dzmaterial";
        const std::string transWhiteMaterialFilePath    = "Assets/Materials/Next/WhiteTransparent.dzmaterial";
        const std::string transRedMaterialFilePath      = "Assets/Materials/Next/RedTransparent.dzmaterial";
        const std::string plasticMaterialFilePath       = "Assets/Materials/Next/Plastic015A.dzmaterial";
        const std::string woodMaterialFilePath          = "Assets/Materials/Next/Wood049.dzmaterial";
        const std::string metalMaterialFilePath         = "Assets/Materials/Next/Metal042A.dzmaterial";

        // Create spheres to test materials
        Entity sphere1 = CreateSphere(deferredPBRMaterialFilePath, {-21, 10, 0}, scene);
        Entity sphere2 = CreateSphere(pureMetalMaterialFilePath, {-15, 10, 0}, scene);
        Entity sphere3 = CreateSphere(transWhiteMaterialFilePath, {-9, 10, 0}, scene);
        Entity sphere4 = CreateSphere(transRedMaterialFilePath, {-3, 10, 0}, scene);
        Entity sphere5 = CreateSphere(plasticMaterialFilePath, {3, 10, 0}, scene);
        Entity sphere6 = CreateSphere(woodMaterialFilePath, {9, 10, 0}, scene);
        Entity sphere7 = CreateSphere(metalMaterialFilePath, {15, 10, 0}, scene);
        Entity sphere8 = CreateSphere(deferredWhiteMaterialFilePath, {21, 10, 0}, scene);

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform               = floor.GetComponent<TransformComponent>();
        floorTransform.Scale               = {100, 1, 100};
        auto& floorMeshFilter              = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType      = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer            = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.MaterialFilePath = deferredWhiteMaterialFilePath;

        // Create 4 point lights

        // red
        // CreatePointLight(glm::vec3(-50, 20, 50), glm::vec3(1, 0, 0), scene);

        // // green
        // CreatePointLight(glm::vec3(-50, 20, -50), glm::vec3(0, 1, 0), scene);

        // // blue
        // CreatePointLight(glm::vec3(50, 20, 50), glm::vec3(0, 0, 1), scene);

        // // yellow
        // CreatePointLight(glm::vec3(50, 20, -50), glm::vec3(1, 1, 0), scene);

        // Load materials
        const std::string instancingMaterialPath = "Assets/Materials/Next/Grass.dzmaterial";

        // Create grass quad instances
        for (size_t i = 0; i < 250; ++i)
        {
            glm::vec4 randomPosition(
                Random::GetRandomFloatRanged(-12, 12), 5, Random::GetRandomFloatRanged(-12, 12), 1);
            CreateQuad(instancingMaterialPath, randomPosition, scene);
        }
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv) TRY
{
    REGISTER_TYPE(EscScript);

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - RenderingSystem-Overview";
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