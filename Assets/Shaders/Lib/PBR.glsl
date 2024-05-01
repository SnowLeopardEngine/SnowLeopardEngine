#ifndef PBR_GLSL

#include "Lib/Math.glsl"

struct PBRMaterial {
    vec3 albedo;
    vec3 emissive;
    float metallic;
    float roughness;
    float ao;
    float opacity;
};

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float DistributionGTR(vec3 N, vec3 H, float roughness, float gamma) {
    float alpha = roughness * roughness;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float alpha2 = alpha * alpha;

    float denom = (NdotH2 * (alpha2 - 1.0) + 1.0);
    denom = PI * pow(denom, gamma);

    float c = alpha2;
    float D = c / denom;

    return D;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float V_GGX(float NdotV, float NdotL, float a) {
    const float a2 = a * a;
    const float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    const float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);
    const float GGX = GGXV + GGXL;

    return GGX > 0.0 ? 0.5 / GGX : 0.0;
}

float V_SmithGGXCorrelated(float NdotV, float NdotL, float roughness) {
    const float a2 = pow(roughness, 4.0);
    const float GGXV = NdotL * sqrt(NdotV * NdotV * (1.0 - a2) + a2);
    const float GGXL = NdotV * sqrt(NdotL * NdotL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

#endif