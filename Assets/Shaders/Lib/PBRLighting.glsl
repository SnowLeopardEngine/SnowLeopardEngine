#ifndef PBR_LIGHTING_GLSL
#define PBR_LIGHTING_GLSL

#include "Common/Light.glsl"
#include "Lib/Shadow.glsl"
#include "Lib/PBR.glsl"
#include "Lib/IBL.glsl"

vec3 calDirectionalLight(DirectionalLight directionalLight, vec3 F0, vec3 N, vec3 V, PBRMaterial material) {
    vec3 L = normalize(-directionalLight.direction);
    vec3 H = normalize(V + L);
    vec3 radiance = directionalLight.color * directionalLight.intensity;

    // cook-torrance brdf
    float gamma = 2.0;
    float NDF = DistributionGTR(N, H, material.roughness, gamma);
    float G = GeometrySmith(N, V, L, material.roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-12;
    vec3 specular = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);

    return (kD * material.albedo / PI + specular) * radiance * NdotL;
}

vec3 calPointLight(PointLight pointLight, vec3 F0, vec3 N, vec3 V, PBRMaterial material, vec3 fragPos) {
    vec3 L = normalize(pointLight.position - fragPos);
    vec3 H = normalize(V + L);
    vec3 radiance = pointLight.color * pointLight.intensity;

    // cook-torrance brdf
    float gamma = 2.0;
    float NDF = DistributionGTR(N, H, material.roughness, gamma);
    float G = GeometrySmith(N, V, L, material.roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - material.metallic;

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 1e-12;
    vec3 specular = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);

    return (kD * material.albedo / PI + specular) * radiance * NdotL;
}

vec3 calIBLAmbient(vec3 diffuseColor, vec3 F0, vec3 N, vec3 V, PBRMaterial material, sampler2D brdfLUT, samplerCube irradianceMap, samplerCube prefilteredEnvMap) {
    float NdotV = dot(N, V);
    LightContribution iblContribution = calIBL(diffuseColor, F0, 0.5, material.roughness, N, V, NdotV, brdfLUT, irradianceMap, prefilteredEnvMap);
    return iblContribution.Diffuse * material.ao + iblContribution.Specular * material.ao;
}

vec3 calPBRLighting(DirectionalLight directionalLight, PointLight pointLights[NUM_MAX_POINT_LIGHT], uint numPointLights, vec3 normal, vec3 viewDir, PBRMaterial material, vec3 fragPos, sampler2D shadowMap, sampler2D brdfLUT, samplerCube irradianceMap, samplerCube prefilteredEnvMap) {
    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);

    vec3 F0 = vec3(0.04);
    const vec3 diffuseColor = mix(material.albedo * (1.0 - F0), vec3(0.0), material.metallic);
    F0 = mix(F0, material.albedo, material.metallic);

    vec3 Lo = vec3(0.0);
    // directional light contribution
    Lo += calDirectionalLight(directionalLight, F0, N, V, material);
    // point lights contribution
    for(uint i = 0; i < numPointLights; ++i) {
        Lo += calPointLight(pointLights[i], F0, N, V, material, fragPos);
    }

    Lo += calIBLAmbient(diffuseColor, F0, N, V, material, brdfLUT, irradianceMap, prefilteredEnvMap);

    vec4 fragPosLightSpace = directionalLight.lightSpaceMatrix * vec4(fragPos, 1.0);
    float shadow = calShadow(fragPosLightSpace, normal, normalize(-directionalLight.direction), shadowMap);

    vec3 ambient = vec3(0.01) * material.albedo * material.ao;
    vec3 color = material.emissive + ambient + (1.0 - shadow) * Lo;

    return color;
}

#endif