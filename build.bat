@echo off

powershell.exe -ExecutionPolicy Bypass -File "Source\CSharpBindings\build.ps1"
IF NOT %ERRORLEVEL% == 0 (
    echo Failed to build C# Bindings!
) ELSE (
    xmake -v -y
)

PAUSE