#!/usr/bin/env sh
# Regenerates the DzShader ANTLR parser (Source/Runtime/src/Function/Rendering/Shader/Grammar/generated) from the
# .g4 grammars with the pinned official antlr-4.13.2-complete.jar. Needs Java; the build itself does not.
set -eu
if [ "$#" -lt 1 ]; then
    echo "Usage: generate_dzshader_parser.sh /path/antlr-4.13.2-complete.jar [java]" >&2
    exit 1
fi
jar=$(cd "$(dirname "$1")" && pwd)/$(basename "$1")
java=${2:-java}
expected=eae2dfa119a64327444672aff63e9ec35a20180dc5b8090b7a6ab85125df4d76
if command -v sha256sum >/dev/null 2>&1; then
    actual=$(sha256sum "$jar" | cut -d ' ' -f 1)
else
    actual=$(shasum -a 256 "$jar" | cut -d ' ' -f 1)
fi
[ "$actual" = "$expected" ] || { echo "Expected the pinned ANTLR 4.13.2 generator." >&2; exit 1; }
root=$(cd "$(dirname "$0")/../.." && pwd)
output="$root/build/.tmp/antlr-generated"
mkdir -p "$output"
cd "$root/Source/Runtime/src/Function/Rendering/Shader/Grammar"
"$java" -jar "$jar" -Dlanguage=Cpp -no-listener -o "$output" dzshader_lexer.g4 dzshader_parser.g4
for file in dzshader_lexer.cpp dzshader_lexer.h dzshader_parser.cpp dzshader_parser.h; do
    # Normalize generated whitespace without changing code or requiring a formatter.
    awk '
        {
            sub(/\r$/, "")
            sub(/[ \t]+$/, "")
            while (match($0, /^[ \t]*\t/)) {
                prefix = substr($0, 1, RLENGTH)
                rest = substr($0, RLENGTH + 1)
                gsub(/\t/, "    ", prefix)
                $0 = prefix rest
            }
            if ($0 == "") { pending++; next }
            while (pending > 0) { print ""; pending-- }
            print
        }
    ' "$output/$file" > "$output/$file.normalized"
    cp "$output/$file.normalized" "generated/$file"
done
