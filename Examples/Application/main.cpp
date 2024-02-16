#include <SnowLeopardEngine/Engine/DesktopApp.h>

using namespace SnowLeopardEngine;

int main()
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - Application";
    DesktopApp app;

    if (!app.Init(initInfo))
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    app.Run();

    return 0;
}