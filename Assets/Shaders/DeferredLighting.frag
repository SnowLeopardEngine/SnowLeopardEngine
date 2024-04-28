#version 450

#include "Common/FrameUniform.glsl"
#include "Lib/PBRLighting.glsl"

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0, binding = 0) uniform sampler2D gPosition;
layout(location = 1, binding = 1) uniform sampler2D gNormal;
layout(location = 2, binding = 2) uniform sampler2D gAlbedo;
layout(location = 3, binding = 3) uniform sampler2D gMetallicRoughnessAO;
layout(location = 4, binding = 4) uniform sampler2D shadowMap;

layout(location = 0) out vec3 FragColor;

void main() {
    PBRMaterial material;

    // Retrieve data from G-Buffer
    vec3 fragPos = texture(gPosition, varingTexCoords).rgb;
    vec3 worldNormal = texture(gNormal, varingTexCoords).rgb;

    vec3 albedo = texture(gAlbedo, varingTexCoords).rgb;
    material.albedo = albedo;

    vec4 metallicRoughnessAO = texture(gMetallicRoughnessAO, varingTexCoords);
    material.metallic = metallicRoughnessAO.r;
    material.roughness = metallicRoughnessAO.g;
    material.ao = metallicRoughnessAO.b;

    vec4 fragPosLightSpace = getLightSpaceMatrix() * vec4(fragPos, 1.0);

    vec3 viewDir = normalize(getViewPos() - fragPos);

    DirectionalLight directionalLight;
    directionalLight.direction = getDirectionalLightDirection();
    directionalLight.color = getDirectionalLightColor();
    directionalLight.intensity = getDirectionalLightIntensity();

    FragColor = CalPBRLighting(directionalLight, worldNormal, viewDir, material, fragPosLightSpace, shadowMap);
}