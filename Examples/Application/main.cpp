#include <SnowLeopardEngine/Engine/DesktopApp.h>

using namespace SnowLeopardEngine;

int main()
{
    DesktopApp app;
    if (!app.Init())
    {
        std::cerr << "Failed to initialize the application!" << std::endl;
        return 1;
    }

    app.Run();

    return 0;
}