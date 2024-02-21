#include "SnowLeopardEngine/Core/Base/Base.h"
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

        // Create a smooth material
        auto smoothMaterial = CreateRef<PhysicsMaterial>(0, 0, 1);
        // Create a sphere with RigidBodyComponent & SphereColliderComponent
        Entity sphere = scene->CreateEntity("Cube");

        sphere.GetComponent<TransformComponent>().Position.y = 10.0f;

        sphere.GetComponent<DampingComponent>().LinearDamping = 5;
        sphere.GetComponent<DampingComponent>().AngularDamping = 10;
        sphere.GetComponent<DampingComponent>().EnableCCD = true;

        sphere.AddComponent<RigidBodyComponent>(1.0f);
        glm::vec3 size1   = {4, 1, 2};
        glm::vec3 offset1 = {0, 0, 0};
        sphere.AddComponent<BoxColliderComponent>(size1, offset1, smoothMaterial);


        Entity capsule = scene->CreateEntity("Capsule");
        capsule.GetComponent<TransformComponent>().Position.y = 20.0f;
        capsule.GetComponent<DampingComponent>().LinearDamping = 5;
        capsule.GetComponent<DampingComponent>().AngularDamping = 10;
        capsule.GetComponent<DampingComponent>().EnableCCD = true;

        capsule.AddComponent<RigidBodyComponent>(1.0f);
        glm::vec3 offset2 = {5, 5, 5};
        capsule.AddComponent<CapsuleColliderComponent>(2.0f, 2.0f, offset2, smoothMaterial);
        
        // Create a floor with RigidBodyComponent & BoxColliderComponent
        Entity floor = scene->CreateEntity("Floor");

        // set it to static, so that rigidBody will be static.
        floor.GetComponent<EntityStatusComponent>().IsStatic = true;
        floor.AddComponent<RigidBodyComponent>();
        glm::vec3 size   = {50, 1, 50};
        glm::vec3 offset = {0, 0, 0};
        floor.AddComponent<BoxColliderComponent>(size, offset, smoothMaterial);
    }

private:
    EngineContext* m_EngineContext;
};

int main()
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - PhysicsSystem";
    DesktopApp app;

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