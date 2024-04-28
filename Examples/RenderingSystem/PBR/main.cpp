#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
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

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_EngineContext = DesktopApp::GetInstance()->GetEngine()->GetContext();

        // Hide mouse cursor
        m_EngineContext->WindowSys->SetHideCursor(true);

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("RenderingSystem-PBR", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox
        // cameraComponent.SkyboxMaterial = DzMaterial::LoadFromPath("Assets/Materials/Skybox001.dzmaterial");

        camera.AddComponent<FreeMoveCameraControllerComponent>();
        camera.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(EscScript));

        // Load materials
        const std::string deferredWhiteMaterialFilePath = "Assets/Materials/Next/White.dzmaterial";
        const std::string deferredPBRMaterialFilePath   = "Assets/Materials/Next/RustedIronPBRDeferred.dzmaterial";

        // Create spheres to test materials
        // Sphere 1
        Entity sphere1 = CreateSphere(deferredPBRMaterialFilePath, {-21, 10, 0}, scene);
        Entity sphere2 = CreateSphere(deferredPBRMaterialFilePath, {-15, 10, 0}, scene);
        Entity sphere3 = CreateSphere(deferredPBRMaterialFilePath, {-9, 10, 0}, scene);
        Entity sphere4 = CreateSphere(deferredPBRMaterialFilePath, {-3, 10, 0}, scene);
        Entity sphere5 = CreateSphere(deferredPBRMaterialFilePath, {3, 10, 0}, scene);
        Entity sphere6 = CreateSphere(deferredPBRMaterialFilePath, {9, 10, 0}, scene);
        Entity sphere7 = CreateSphere(deferredPBRMaterialFilePath, {15, 10, 0}, scene);
        Entity sphere8 = CreateSphere(deferredPBRMaterialFilePath, {21, 10, 0}, scene);

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform               = floor.GetComponent<TransformComponent>();
        floorTransform.Scale               = {100, 1, 100};
        auto& floorMeshFilter              = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType      = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer            = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.MaterialFilePath = deferredWhiteMaterialFilePath;
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv) TRY
{
    REGISTER_TYPE(EscScript);

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - RenderingSystem-PBR";
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