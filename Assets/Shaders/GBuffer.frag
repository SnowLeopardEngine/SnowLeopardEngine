#version 450

#include "Lib/PBR.glsl"

layout(location = 0) in vec2 varingTexCoords;
layout(location = 1) flat in int varingEntityID;
layout(location = 2) in vec3 worldNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec3 gMetallicRoughnessAO;
layout(location = 4) out int gEnityID;

layout(location = 0) uniform vec4 albedo;
layout(location = 1, binding = 0) uniform sampler2D albedoMap;
layout(location = 2) uniform int useAlbedoMap;
layout(location = 3) uniform float metallic;
layout(location = 4, binding = 1) uniform sampler2D metallicMap;
layout(location = 5) uniform int useMetallicMap;
layout(location = 6) uniform float roughness;
layout(location = 7, binding = 2) uniform sampler2D roughnessMap;
layout(location = 8) uniform int useRoughnessMap;
layout(location = 9) uniform float ao;
layout(location = 10, binding = 3) uniform sampler2D aoMap;
layout(location = 11) uniform int useAoMap;

void main() {
    PBRMaterial material;
    material.albedo = (1 - useAlbedoMap) * albedo.rgb + useAlbedoMap * texture(albedoMap, varingTexCoords).rgb;
    material.metallic = (1 - useMetallicMap) * metallic + useMetallicMap * texture(metallicMap, varingTexCoords).r;
    material.roughness = (1 - useRoughnessMap) * roughness + useRoughnessMap * texture(roughnessMap, varingTexCoords).r;
    material.ao = (1 - useAoMap) * ao + useAoMap * texture(aoMap, varingTexCoords).r;

    gPosition = fragPos;
    gNormal = normalize(worldNormal);
    gAlbedo.rgb = material.albedo;
    gMetallicRoughnessAO.r = material.metallic;
    gMetallicRoughnessAO.g = material.roughness;
    gMetallicRoughnessAO.b = ao;
    gEnityID.r = varingEntityID;
}