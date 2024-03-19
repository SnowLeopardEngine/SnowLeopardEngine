#include "LegacySceneUniform.glsl"
#include "LegacyShadow.glsl"

#define PI 3.1415926535
#define MAX_POINT_LIGHT_COUNT 10

struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
};

struct PointLight {
    vec3 position;
    float intensity;
    vec3 color;
};

struct PBRMaterial {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
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

#ifdef FORWARD_LIGHTING

vec3 CalPBRLighting(DirectionalLight directionalLight, vec3 normal, vec3 viewDir, PBRMaterial material, vec4 fragPosLightSpace, sampler2D shadowMap) {
    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, material.albedo, material.metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    // Only care about 1 directional light for now.

    vec3 L = normalize(-directionalLight.direction);
    vec3 H = normalize(V + L);
    vec3 radiance = directionalLight.color * directionalLight.intensity;

    // cook-torrance brdf
    float gamma = 2.0f;
    float NDF = DistributionGTR(N, H, material.roughness, gamma);
    float G = GeometrySmith(N, V, L, material.roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);

    Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;

    vec3 ambient = vec3(0.03) * material.albedo * material.ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    return color;
}
#endif