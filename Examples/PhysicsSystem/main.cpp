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
        auto scene = m_EngineContext->SceneMngr->CreateScene("NativeScripting", true);

        // Create an entity with RigidBodyComponent & SphereColliderComponent
        Entity entity = scene->CreateEntity("Sphere");
        entity.AddComponent<RigidBodyComponent>();
        entity.AddComponent<SphereColliderComponent>();
    }

private:
    EngineContext* m_EngineContext;
};

int main()
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - NativeScripting";
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