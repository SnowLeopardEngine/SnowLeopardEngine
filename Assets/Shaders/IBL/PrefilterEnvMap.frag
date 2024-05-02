#version 450

#include "Lib/Math.glsl"

layout(location = 0) in vec3 fragPos;

layout(location = 0) out vec3 FragColor;

layout(binding = 0) uniform samplerCube envMap;
layout(location = 1) uniform float roughness;

float D_GGX(float NdotH, float roughness) {
    float a = NdotH * roughness;
    float k = roughness / (1.0 - NdotH * NdotH + a * a);
    return k * k * (1.0 / PI);
}

void main() {
    const float width = textureSize(envMap, 0).x;
    const float a = roughness * roughness;

    const vec3 N = normalize(fragPos);
    const mat3 TBN = generateTBN(N);

    float weight = 0.0;
    vec3 color = vec3(0.0);

    const uint kNumSamples = 1024;
    for(uint i = 0; i < kNumSamples; ++i) {
        const vec2 Xi = hammersley2D(i, kNumSamples);

        const float cosTheta = clamp(sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y)), 0.0, 1.0);
        const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
        const float phi = 2.0 * PI * Xi.x;

        const float pdf = D_GGX(cosTheta, a) / 4.0;

        const vec3 localSpaceDirection = normalize(vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta));
        const vec3 H = TBN * localSpaceDirection;

        const vec3 V = N;
        const vec3 L = normalize(reflect(-V, H));
        const float NdotL = dot(N, L);

        if(NdotL > 0.0) {
            float lod = 0.5 * log2(6.0 * float(width) * float(width) /
                (float(kNumSamples) * pdf));
            if(roughness == 0.0)
                lod = 0.0;
            const vec3 sampleColor = textureLod(envMap, L, lod).rgb;
            if(!any(isinf(sampleColor))) {
                color += sampleColor * NdotL;
                weight += NdotL;
            }
        }
    }

    FragColor = color / ((weight != 0.0) ? weight : float(kNumSamples));
}