#version 450

#include "Common/FrameUniform.glsl"
#include "Common/LightUniform.glsl"
#include "Lib/PBRLighting.glsl"
#include "Lib/Color.glsl"

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec4 grassTarget;

layout(binding = 1) uniform sampler2D albedoMap;

void main() {
    vec4 grassColor = texture(albedoMap, varingTexCoords);
    if (grassColor.a < 0.5)
    {
        discard;
    }
    // The grass target is blitted into the linear HDR scene color, so decode the sRGB texture first.
    grassTarget = vec4(gammaToLinear(grassColor.rgb), 1.0);
}