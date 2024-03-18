#!/bin/bash

currentScriptDirectory="$(dirname "$(readlink -f "$0")")"
buildBindingsScriptPath="$currentScriptDirectory/Source/CSharpBindings/build.sh"

bash "$buildBindingsScriptPath"

if [ $? -ne 0 ]; then
    echo "Failed to build C# Bindings!"
else
    xmake -v -y
fi