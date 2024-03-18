#include "SnowLeopardEngine/Function/CSharpScripting/CSharpScriptingSystem.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/Log/LogTypeDef.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <Coral/HostInstance.hpp>
#include <Coral/MessageLevel.hpp>
#include <Coral/String.hpp>

namespace SnowLeopardEngine
{
    static LogLevel CoralMessageLevelToOurLevel(Coral::MessageLevel level)
    {
        switch (level)
        {
            case Coral::MessageLevel::Info:
                return LogLevel::Info;
            case Coral::MessageLevel::Warning:
                return LogLevel::Warn;
            case Coral::MessageLevel::Error:
                return LogLevel::Error;
            case Coral::MessageLevel::All:
                return LogLevel::Trace;
                break;
        }
    }

    static void MessageCallback(std::string_view message, Coral::MessageLevel level)
    {
        g_EngineContext->LogSys->Message(true, CoralMessageLevelToOurLevel(level), message);
    }

    static void ExceptionCallback(std::string_view exception)
    {
        SNOW_LEOPARD_CORE_CRITICAL("[CSharpScriptingSystem] Unhandled native exception: {0}", exception);
    }

    CSharpScriptingSystem::CSharpScriptingSystem()
    {
        // Load Coral
        auto                exeDir   = FileSystem::GetExecutableDirectory();
        auto                coralDir = exeDir.string();
        Coral::HostSettings settings = {};
        settings.CoralDirectory      = coralDir;
        settings.ExceptionCallback   = ExceptionCallback;
        settings.MessageCallback     = MessageCallback;

        Coral::HostInstance hostInstance;
        if (!hostInstance.Initialize(settings))
        {
            m_State = SystemState::Error;
            SNOW_LEOPARD_CORE_ERROR("[CSharpScriptingSystem] Failed to initialize Coral assembly!");
            return;
        }

        auto loadContext = hostInstance.CreateAssemblyLoadContext("ExampleContext");

        auto  assemblyPath = exeDir / "SnowLeopardEngine.dll";
        auto& assembly     = loadContext.LoadAssembly(assemblyPath.string());

        // Echo test
        auto& testsType = assembly.GetType("SnowLeopardEngine.Tests");

        // Call the static method "Echo" with value "Hello, C# .NET 8.0 Scripting!"
        auto echoMsg = Coral::String::New("Hello, C# .NET 8.0 Scripting!");
        testsType.InvokeStaticMethod("Echo", echoMsg);
        Coral::String::Free(echoMsg);

        SNOW_LEOPARD_CORE_INFO("[CSharpScriptingSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    CSharpScriptingSystem::~CSharpScriptingSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[CSharpScriptingSystem] Shutting Down...");
        m_State = SystemState::ShutdownOk;
    }
} // namespace SnowLeopardEngine