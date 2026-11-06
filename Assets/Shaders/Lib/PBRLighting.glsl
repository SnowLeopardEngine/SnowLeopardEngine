#ifndef PBR_LIGHTING_GLSL
#define PBR_LIGHTING_GLSL

#include "Common/Light.glsl"
#include "Lib/PBR.glsl"
#include "Lib/IBL.glsl"
#include "Lib/CSM.glsl"

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
    float NdotV = clamp(dot(N, V), 0.0, 1.0);
    LightContribution iblContribution = calIBL(diffuseColor, F0, 0.5, material.roughness, N, V, NdotV, brdfLUT, irradianceMap, prefilteredEnvMap);
    return iblContribution.Diffuse * material.ao + iblContribution.Specular * material.ao;
}

vec3 calPBRLighting(DirectionalLight directionalLight, PointLight pointLights[NUM_MAX_POINT_LIGHT], uint numPointLights, vec3 normal, PBRMaterial material, vec3 fragPos, vec3 viewPos, sampler2DArrayShadow cascadedShadowMaps, sampler2D brdfLUT, samplerCube irradianceMap, samplerCube prefilteredEnvMap) {
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);

    vec3 F0 = vec3(0.04);
    const vec3 diffuseColor = mix(material.albedo * (1.0 - F0), vec3(0.0), material.metallic);
    F0 = mix(F0, material.albedo, material.metallic);

    // directional light contribution (the only light the cascaded shadow maps cover)
    vec3 directLo = calDirectionalLight(directionalLight, F0, N, V, material);

    // point lights contribution (no shadows for point lights yet)
    vec3 pointLo = vec3(0.0);
    for(uint i = 0; i < numPointLights; ++i) {
        pointLo += calPointLight(pointLights[i], F0, N, V, material, fragPos);
    }

    // image-based (sky) lighting: occluded by AO, not by the sun's shadow
    vec3 iblLo = calIBLAmbient(diffuseColor, F0, N, V, material, brdfLUT, irradianceMap, prefilteredEnvMap);

    // select cascade layer
    vec4 fragPosViewSpace = getViewMatrix() * vec4(fragPos, 1.0);
    uint cascadeIndex = selectCascadeIndex(fragPosViewSpace.xyz);

    // calculate shadow
    float shadow = calShadow(cascadeIndex, fragPos, normal, directionalLight.direction, cascadedShadowMaps);

    vec3 ambient = vec3(0.01) * material.albedo * material.ao;
    vec3 color = material.emissive + ambient + iblLo + pointLo + (1.0 - shadow) * directLo;

    return color;
}

#endif