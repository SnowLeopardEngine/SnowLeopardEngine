#include <SnowLeopardEngine/Engine/DesktopApp.h>

using namespace SnowLeopardEngine;

class Tickable1 final : public Tickable
{
public:
    virtual void OnTick(float deltaTime) override final
    {
        SNOW_LEOPARD_INFO("[Tickable1] OnTick, deltaTime = {0}", deltaTime);
    }

    virtual void OnFixedTick() override final { SNOW_LEOPARD_INFO("[Tickable1] OnFixedTick"); }
};

class Tickable2 final : public Tickable
{
public:
    virtual void OnTick(float deltaTime) override final
    {
        SNOW_LEOPARD_INFO("[Tickable2] OnTick, deltaTime = {0}", deltaTime);
    }

    virtual void OnFixedTick() override final { SNOW_LEOPARD_INFO("[Tickable2] OnFixedTick"); }
};

int main()
{
    DesktopApp app;

    if (!app.Init())
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    // add tickables to the engine
    auto t1 = CreateRef<Tickable1>();
    auto t2 = CreateRef<Tickable2>();
    app.GetEngine()->AddTickable(t1);
    app.GetEngine()->AddTickable(t2);

    app.Run();

    return 0;
}