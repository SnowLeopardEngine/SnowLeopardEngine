$currentScriptDirectory = $PSScriptRoot

$csProjPath = Join-Path -Path $currentScriptDirectory -ChildPath "SnowLeopardEngine/SnowLeopardEngine.csproj"
$outputPath = Join-Path -Path $currentScriptDirectory -ChildPath "../../build/assembly/SnowLeopardEngine"

dotnet build $csProjPath -c Release -o $outputPath