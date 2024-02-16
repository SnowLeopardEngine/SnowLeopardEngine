#include <SnowLeopardEngine/Engine/DesktopApp.h>

using namespace SnowLeopardEngine;

class CustomLifeTime final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final
    {
        SNOW_LEOPARD_INFO("[CustomLifeTime] OnLoad");
        DesktopApp::GetInstance()->GetEngine()->GetContext()->AudioSys->Play("sounds/ImperialMarch60.wav");
    }

    virtual void OnTick(float deltaTime) override final
    {
        SNOW_LEOPARD_INFO("[CustomLifeTime] OnTick, deltaTime = {0}", deltaTime);
    }

    virtual void OnFixedTick() override final { SNOW_LEOPARD_INFO("[CustomLifeTime] OnFixedTick"); }

    virtual void OnUnload() override final { SNOW_LEOPARD_INFO("[CustomLifeTime] OnUnload"); }
};

int main()
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - AudioSystem";
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