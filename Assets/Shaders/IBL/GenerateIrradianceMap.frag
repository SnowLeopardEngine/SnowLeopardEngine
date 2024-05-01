#version 450

#include "Lib/Math.glsl"

layout(location = 0) in vec3 fragPos;

layout(location = 0) out vec3 FragColor;

layout(binding = 0) uniform samplerCube envMap;

void main() {
    const uint width = textureSize(envMap, 0).x;

    const vec3 N = normalize(fragPos);
    const mat3 TBN = generateTBN(N);

    vec3 color = vec3(0.0);

    const uint kNumSamples = 2048;
    for(uint i = 0; i < kNumSamples; ++i) {
        const vec2 Xi = hammersley2D(i, kNumSamples);
        const float cosTheta = sqrt(1.0 - Xi.y);
        const float sinTheta = sqrt(Xi.y);
        const float phi = 2.0 * PI * Xi.x;

        const vec3 localSpaceDirection = normalize(vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
        const vec3 H = TBN * localSpaceDirection;
        const float pdf = cosTheta / PI;

        float lod = 0.5 * log2(6.0 * float(width) * float(width) /
            (float(kNumSamples) * pdf));
        const vec3 sampleColor = textureLod(envMap, H, lod).rgb;
        if(!any(isinf(sampleColor))) {
            color += sampleColor;
        }
    }

    FragColor = color / float(kNumSamples);
}