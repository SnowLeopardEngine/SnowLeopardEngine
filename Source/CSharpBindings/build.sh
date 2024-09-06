#!/bin/bash

currentScriptDirectory="$(dirname "$(readlink -f "$0")")"

csProjPath="$currentScriptDirectory/SnowLeopardEngine/SnowLeopardEngine.csproj"
outputPath="$currentScriptDirectory/../../build/assembly/SnowLeopardEngine"

dotnet build "$csProjPath" -c Release -o "$outputPath"

if [ $? -eq 0 ]; then
    exit 0
else
    exit 1
fi