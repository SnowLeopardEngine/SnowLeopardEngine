#include <SnowLeopardEngine/Engine/DesktopApp.h>

using namespace SnowLeopardEngine;

int main(int argc, char** argv)
{
    DesktopAppInitInfo initInfo {};
    initInfo.Engine.Window.Title = "Example - Application";
    DesktopApp app(argc, argv);

    if (!app.Init(initInfo))
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    app.Run();

    return 0;
}