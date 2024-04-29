#ifndef PBR_LIGHTING_GLSL
#define PBR_LIGHTING_GLSL

#include "Common/Light.glsl"
#include "Lib/Shadow.glsl"
#include "Lib/PBR.glsl"

vec3 CalDirectionalLight(DirectionalLight directionalLight, vec3 F0, vec3 N, vec3 V, PBRMaterial material) {
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

    return (kD * material.albedo / PI + specular) * radiance * NdotL;
}

vec3 CalPointLight(PointLight pointLight, vec3 F0, vec3 N, vec3 V, PBRMaterial material, vec3 fragPos) {
    vec3 L = normalize(pointLight.position - fragPos);
    vec3 H = normalize(V + L);
    vec3 radiance = pointLight.color;

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

    return (kD * material.albedo / PI + specular) * radiance * NdotL;
}

vec3 CalPBRLighting(DirectionalLight directionalLight, PointLight pointLights[NUM_MAX_POINT_LIGHT], uint numPointLights, vec3 normal, vec3 viewDir, PBRMaterial material, vec3 fragPos, sampler2D shadowMap) {
    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, material.albedo, material.metallic);

    vec3 Lo = vec3(0.0);
    // directional light contribution
    Lo += CalDirectionalLight(directionalLight, F0, N, V, material);
    // point lights contribution
    for (uint i = 0; i < numPointLights; ++i)
    {
        Lo += CalPointLight(pointLights[i], F0, N, V, material, fragPos);
    }

    vec4 fragPosLightSpace = directionalLight.lightSpaceMatrix * vec4(fragPos, 1.0);
    float shadow = ShadowCalculation(fragPosLightSpace, normal, normalize(-directionalLight.direction), shadowMap);

    vec3 ambient = vec3(0.03) * material.albedo * material.ao;
    vec3 color = ambient + (1.0 - shadow) * Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    return color;
}

#endif