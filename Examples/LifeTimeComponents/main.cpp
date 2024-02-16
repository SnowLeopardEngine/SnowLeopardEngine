#include <SnowLeopardEngine/Engine/DesktopApp.h>

using namespace SnowLeopardEngine;

class LifeTimeA final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final { SNOW_LEOPARD_INFO("[LifeTimeA] OnLoad"); }

    virtual void OnTick(float deltaTime) override final
    {
        SNOW_LEOPARD_INFO("[LifeTimeA] OnTick, deltaTime = {0}", deltaTime);
    }

    virtual void OnFixedTick() override final { SNOW_LEOPARD_INFO("[LifeTimeA] OnFixedTick"); }

    virtual void OnUnload() override final { SNOW_LEOPARD_INFO("[LifeTimeA] OnUnload"); }
};

class LifeTimeB final : public LifeTimeComponent
{
public:
    virtual void OnLoad() override final { SNOW_LEOPARD_INFO("[LifeTimeB] OnLoad"); }

    virtual void OnTick(float deltaTime) override final
    {
        SNOW_LEOPARD_INFO("[LifeTimeB] OnTick, deltaTime = {0}", deltaTime);
    }

    virtual void OnFixedTick() override final { SNOW_LEOPARD_INFO("[LifeTimeB] OnFixedTick"); }

    virtual void OnUnload() override final { SNOW_LEOPARD_INFO("[LifeTimeB] OnUnload"); }
};

int main()
{
    DesktopApp app;

    if (!app.Init())
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    // add lifeTimeComponents to the engine
    auto l1 = CreateRef<LifeTimeA>();
    auto l2 = CreateRef<LifeTimeB>();
    app.GetEngine()->AddLifeTimeComponent(l1);
    app.GetEngine()->AddLifeTimeComponent(l2);

    app.Run();

    return 0;
}