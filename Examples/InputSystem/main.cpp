#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include <SnowLeopardEngine/Engine/DesktopApp.h>
#include <SnowLeopardEngine/Function/Scene/Entity.h>

using namespace SnowLeopardEngine;

class MyScriptInstance : public NativeScriptInstance
{
public:
    virtual void OnLoad() override final { SNOW_LEOPARD_INFO("[MyScriptInstance] OnLoad"); }

    virtual void OnTick(float deltaTime) override final
    {
        auto& inputSystem = DesktopApp::GetInstance()->GetEngine()->GetContext()->InputSys;
        if (inputSystem->GetKey(KeyCode::Space))
        {
            SNOW_LEOPARD_INFO("[MyScriptInstance] Space Key!");
        }

        if (inputSystem->GetKeyDown(KeyCode::Space))
        {
            SNOW_LEOPARD_INFO("[MyScriptInstance] Space KeyDown!");
        }

        if (inputSystem->GetKeyUp(KeyCode::Space))
        {
            SNOW_LEOPARD_INFO("[MyScriptInstance] Space KeyUp!");
        }

        if (inputSystem->GetMouseButton(MouseCode::ButtonLeft))
        {
            SNOW_LEOPARD_INFO("[MyScriptInstance] Left MouseButton!");
        }

        if (inputSystem->GetMouseButtonDown(MouseCode::ButtonLeft))
        {
            SNOW_LEOPARD_INFO("[MyScriptInstance] Left MouseButtonDown!");
        }

        if (inputSystem->GetMouseButtonUp(MouseCode::ButtonLeft))
        {
            SNOW_LEOPARD_INFO("[MyScriptInstance] Left MouseButtonUp!");
        }
    }
};

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        m_EngineContext = DesktopApp::GetInstance()->GetEngine()->GetContext();

        // Create a scene and set active
        auto scene = m_EngineContext->SceneMngr->CreateScene("InputSystem", true);

        // Create an entity with NativeScriptingComponent
        Entity entity = scene->CreateEntity("Entity1");
        entity.AddComponent<NativeScriptingComponent>(NAME_OF_TYPE(MyScriptInstance));
    }

private:
    EngineContext* m_EngineContext;
};

int main(int argc, char** argv)
{
    REGISTER_TYPE(MyScriptInstance);

    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - InputSystem";
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