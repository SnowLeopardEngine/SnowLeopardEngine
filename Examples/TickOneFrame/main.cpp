#include <SnowLeopardEngine/Engine/Engine.h>

using namespace SnowLeopardEngine;

int main()
{
    Engine engine;
    if (!engine.Init())
    {
        std::cerr << "Failed to init engine!" << std::endl;
        return 1;
    }

    engine.TickOneFrame(0);
    engine.Shutdown();

    return 0;
}