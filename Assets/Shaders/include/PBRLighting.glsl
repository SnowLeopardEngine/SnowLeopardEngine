#include "LegacySceneUniform.glsl"
#include "LegacyShadow.glsl"
#include "PBR.glsl"

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
    float gamma = 4.0;
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

    float shadow = ShadowCalculation(fragPosLightSpace, shadowMap);

    vec3 ambient = vec3(0.03) * material.albedo * material.ao;
    vec3 color = ambient + (1.0 - shadow) * Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    return color;
}