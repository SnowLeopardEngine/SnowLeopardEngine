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
    DesktopApp app;

    if (!app.Init())
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    // add a custom lifeTimeComponent to the engine
    app.GetEngine()->AddLifeTimeComponent(CreateRef<CustomLifeTime>());

    app.PostInit();
    app.Run();

    return 0;
}