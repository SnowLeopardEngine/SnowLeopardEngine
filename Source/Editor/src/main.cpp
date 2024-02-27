#include "SnowLeopardEditor/EditorApp.h"

using namespace SnowLeopardEngine::Editor;

int main(int argc, char** argv)
{
    EditorAppInitInfo initInfo    = {};
    initInfo.Engine.Window.Title  = "SnowLeopardEditor";
    initInfo.Engine.Window.Width  = 1440;
    initInfo.Engine.Window.Height = 810;

    EditorApp app(argc, argv);

    if (!app.Init(initInfo))
    {
        std::cerr << "Failed to initialize the editor application!" << std::endl;
        return 1;
    }

    if (!app.PostInit())
    {
        std::cerr << "Failed to post initialize the editor application!" << std::endl;
        return 1;
    }

    app.Run();

    return 0;
}