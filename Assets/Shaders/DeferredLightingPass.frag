#version 450

#include "Common/FrameUniform.glsl"
#include "Common/LightUniform.glsl"
#include "Lib/PBRLighting.glsl"

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec3 FragColor;

layout(location = 0, binding = 0) uniform sampler2D gPosition;
layout(location = 1, binding = 1) uniform sampler2D gNormal;
layout(location = 2, binding = 2) uniform sampler2D gAlbedo;
layout(location = 3, binding = 3) uniform sampler2D gEmissive;
layout(location = 4, binding = 4) uniform sampler2D gMetallicRoughnessAO;
layout(location = 5, binding = 5) uniform sampler2D shadowMap;
layout(location = 6, binding = 6) uniform sampler2D brdfLUT;
layout(location = 7, binding = 7) uniform samplerCube irradianceMap;
layout(location = 8, binding = 8) uniform samplerCube prefilteredEnvMap;
layout(location = 9, binding = 9) uniform sampler2D ssao;

void main() {
    PBRMaterial material;

    // Retrieve data from G-Buffer
    vec3 fragPos = texture(gPosition, varingTexCoords).rgb;
    vec3 worldNormal = texture(gNormal, varingTexCoords).rgb;

    vec3 albedo = texture(gAlbedo, varingTexCoords).rgb;
    material.albedo = albedo;

    vec3 emissive = texture(gEmissive, varingTexCoords).rgb;
    material.emissive = emissive;

    vec4 metallicRoughnessAO = texture(gMetallicRoughnessAO, varingTexCoords);
    material.metallic = metallicRoughnessAO.r;
    material.roughness = metallicRoughnessAO.g;
    material.ao = metallicRoughnessAO.b * texture(ssao, varingTexCoords).r; // combine AO texture & SSAO

    vec3 viewDir = normalize(getViewPos() - fragPos);
    FragColor = calPBRLighting(getDirectionalLight(), getPointLights(), getNumPointLights(), worldNormal, viewDir, material, fragPos, shadowMap, brdfLUT, irradianceMap, prefilteredEnvMap);
}