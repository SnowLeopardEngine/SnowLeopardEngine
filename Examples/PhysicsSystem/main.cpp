#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsMaterial.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_EngineContext = DesktopApp::GetInstance()->GetEngine()->GetContext();

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("PhysicsSystem", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 15, 20};
        camera.AddComponent<CameraComponent>();

        // Create a smooth material
        auto smoothMaterial = CreateRef<PhysicsMaterial>(0, 0, 1);
        // Create a sphere with RigidBodyComponent & SphereColliderComponent
        Entity sphere = scene->CreateEntity("Sphere");

        auto& sphereTransform      = sphere.GetComponent<TransformComponent>();
        sphereTransform.Position.y = 10.0f;

        sphere.AddComponent<RigidBodyComponent>(1.0f);
        sphere.AddComponent<SphereColliderComponent>(smoothMaterial);
        auto& sphereMeshFilter         = sphere.AddComponent<MeshFilterComponent>();
        sphereMeshFilter.PrimitiveType = MeshPrimitiveType::Sphere;
        auto& sphereMeshRenderer       = sphere.AddComponent<MeshRendererComponent>();
        sphereMeshRenderer.BaseColor   = {0, 1, 0, 1}; // Green

        // Create a floor with RigidBodyComponent & BoxColliderComponent
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform = floor.GetComponent<TransformComponent>();
        floorTransform.Scale = {50, 0.1, 50};

        // set it to static, so that rigidBody will be static.
        floor.GetComponent<EntityStatusComponent>().IsStatic = true;
        floor.AddComponent<RigidBodyComponent>();
        floor.AddComponent<BoxColliderComponent>(smoothMaterial);
        auto& floorMeshFilter         = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer       = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.BaseColor   = {1, 0, 0, 1}; // Red
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - PhysicsSystem";
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