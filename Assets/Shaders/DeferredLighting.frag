#version 450

#include "PBRLighting.glsl"

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) uniform mat4 lightSpaceMatrix;
layout(location = 1) uniform sampler2D gPosition;
layout(location = 2) uniform sampler2D gNormal;
layout(location = 3) uniform sampler2D gAlbedo;
layout(location = 4) uniform sampler2D gMetallicRoughnessAO;
layout(location = 5) uniform sampler2D gEntityID;
layout(location = 6) uniform sampler2D shadowMap;
layout(location = 7) uniform DirectionalLight directionalLight;

layout(location = 0) out vec4 color0;
layout(location = 1) out int color1;

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

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

    vec3 viewDir = normalize(viewPos - fragPos);

    color0 = vec4(CalPBRLighting(directionalLight, worldNormal, viewDir, material, fragPosLightSpace, shadowMap), 1);
    color1 = int(texture(gEntityID, varingTexCoords).r);
}