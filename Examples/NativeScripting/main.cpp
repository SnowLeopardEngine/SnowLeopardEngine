#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class MyScriptInstance : public NativeScriptInstance
{
public:
    virtual void OnLoad() override final { SNOW_LEOPARD_INFO("[MyScriptInstance] OnLoad"); }

    virtual void OnTick(float deltaTime) override final
    {
        SNOW_LEOPARD_INFO("[MyScriptInstance] OnTick, deltaTime={0}", deltaTime);
        SNOW_LEOPARD_INFO("[MyScriptInstance] Current Health={0}", m_Health);
        m_Health -= deltaTime;
        if (m_Health <= 0)
        {
            SNOW_LEOPARD_INFO("[MyScriptInstance] You died!");
            DesktopApp::GetInstance()->Quit();
        }
    }

    virtual void OnFixedTick() override final { SNOW_LEOPARD_INFO("[MyScriptInstance] OnFixedTick"); }

    virtual void OnUnload() override final { SNOW_LEOPARD_INFO("[MyScriptInstance] OnUnload"); }

private:
    float m_Health = 5.0f;
};

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_EngineContext = DesktopApp::GetInstance()->GetEngine()->GetContext();

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("NativeScripting", true);

        // Create an entity with NativeScriptingComponent
        Entity entity         = scene->CreateEntity("Entity1");
        auto   scriptInstance = CreateRef<MyScriptInstance>();
        entity.AddComponent<NativeScriptingComponent>(scriptInstance);
    }

private:
    EngineContext* m_EngineContext;
};

int main()
{
    DesktopApp app;

    if (!app.Init())
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