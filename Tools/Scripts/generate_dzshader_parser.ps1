# Regenerates the DzShader ANTLR parser (Source/Runtime/src/Function/Rendering/Shader/Grammar/generated) from the
# .g4 grammars with the pinned official antlr-4.13.2-complete.jar. Needs Java; the build itself does not.
param(
    [Parameter(Mandatory = $true)][string]$Jar,
    [string]$Java = "java"
)
$ErrorActionPreference = "Stop"
$expected = "EAE2DFA119A64327444672AFF63E9EC35A20180DC5B8090B7A6AB85125DF4D76"
$generator = (Resolve-Path -LiteralPath $Jar).Path
if ((Get-FileHash -LiteralPath $generator -Algorithm SHA256).Hash -ne $expected) {
    throw "Expected the official antlr-4.13.2-complete.jar with its pinned SHA256."
}
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$grammar = Join-Path $root "Source/Runtime/src/Function/Rendering/Shader/Grammar"
$output = Join-Path $root "build/.tmp/antlr-generated"
New-Item -ItemType Directory -Path $output -Force | Out-Null
Push-Location $grammar
try {
    & $Java -jar $generator -Dlanguage=Cpp -no-listener -o $output dzshader_lexer.g4 dzshader_parser.g4
    if ($LASTEXITCODE -ne 0) { throw "ANTLR generation failed." }
    foreach ($file in @("dzshader_lexer.cpp", "dzshader_lexer.h", "dzshader_parser.cpp", "dzshader_parser.h")) {
        # Preserve generated code while normalizing whitespace for checked-in sources.
        $contents = [System.IO.File]::ReadAllText((Join-Path $output $file))
        $contents = $contents.Replace(([string][char]13 + [char]10), [string][char]10)
        $contents = [regex]::Replace($contents, '^[\t ]+', {
            param($match)
            $match.Value.Replace([string][char]9, "    ")
        }, [System.Text.RegularExpressions.RegexOptions]::Multiline)
        $contents = [regex]::Replace($contents, '[ \t]+$', '', [System.Text.RegularExpressions.RegexOptions]::Multiline)
        $contents = $contents.TrimEnd([char[]]@([char]10)) + [char]10
        [System.IO.File]::WriteAllText((Join-Path $grammar "generated/$file"), $contents,
                                     [System.Text.UTF8Encoding]::new($false))
    }
} finally {
    Pop-Location
}
