#version 450

#include "Lib/PBR.glsl"

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec2 FragColor;

void main() {
    const float NdotV = varingTexCoords.x;
    const vec3 N = vec3(0.0, 0.0, 1.0);
    const vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    const float roughness = varingTexCoords.y;

    float A = 0.0;
    float B = 0.0;

    const uint kNumSamples = 1024;
    for(uint i = 0; i < kNumSamples; ++i) {
        const vec2 Xi = hammersley2D(i, kNumSamples);
        const vec3 H = importanceSampleGGX(Xi, roughness, N);
        const vec3 L = 2.0 * dot(V, H) * H - V;

        const float NdotL = clamp(L.z, 0.0, 1.0);
        if(NdotL > 0.0) {
            const float NdotH = clamp(H.z, 0.0, 1.0);
            const float VdotH = clamp(dot(V, H), 0.0, 1.0);
            const float V_pdf = V_SmithGGXCorrelated(NdotV, NdotL, roughness) * VdotH * NdotL / NdotH;

            const float Fc = pow(1.0 - VdotH, 5.0);
            A += (1.0 - Fc) * V_pdf;
            B += Fc * V_pdf;
        }
    }

    FragColor = 4.0 * vec2(A, B) / float(kNumSamples);
}