#version 450

#include "Lib/Math.glsl"

layout(location = 0) in vec3 fragPos;

layout(binding = 0) uniform sampler2D equirectangularMap;
layout(location = 1) uniform bool invertY = false;

vec2 SampleSphericalMap(vec3 dir) {
    vec2 v = vec2(atan(dir.z, dir.x), asin(dir.y));
    v *= vec2(1.0 / TWO_PI, 1.0 / PI); // -> [-0.5, 0.5]
    return v + 0.5;                    // -> [0.0, 1.0]
}

layout(location = 0) out vec3 FragColor;
void main() {
    vec2 texCoord = SampleSphericalMap(normalize(fragPos));
    if(invertY)
        texCoord.y *= -1.0;
    texCoord = clamp(texCoord, 0.0, 1.0);
    vec3 sampleColor = textureLod(equirectangularMap, texCoord, 0.0).rgb;

    // Gamma-Correction
    sampleColor = sampleColor / (sampleColor + vec3(1.0));
    sampleColor = pow(sampleColor, vec3(1.0/2.2));

    FragColor = sampleColor;
}