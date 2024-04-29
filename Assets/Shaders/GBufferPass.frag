#version 450

#include "Lib/PBR.glsl"
#include "Lib/Color.glsl"

// https://www.khronos.org/opengl/wiki/Early_Fragment_Test
layout(early_fragment_tests) in;

layout(location = 0) in vec2 varingTexCoords;
layout(location = 1) flat in int varingEntityID;
layout(location = 2) in vec3 worldNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec3 gAlbedo;
layout(location = 3) out vec3 gEmissive;
layout(location = 4) out vec3 gMetallicRoughnessAO;
layout(location = 5) out int gEnityID;

layout(location = 0) uniform vec4 albedo;
layout(location = 1, binding = 0) uniform sampler2D albedoMap;
layout(location = 2) uniform int useAlbedoMap;
layout(location = 3) uniform vec4 emissive;
layout(location = 4, binding = 0) uniform sampler2D emissiveMap;
layout(location = 5) uniform int useEmissiveMap;
layout(location = 6) uniform float metallic;
layout(location = 7, binding = 1) uniform sampler2D metallicMap;
layout(location = 8) uniform int useMetallicMap;
layout(location = 9) uniform float roughness;
layout(location = 10, binding = 2) uniform sampler2D roughnessMap;
layout(location = 11) uniform int useRoughnessMap;
layout(location = 12) uniform float ao;
layout(location = 13, binding = 3) uniform sampler2D aoMap;
layout(location = 14) uniform int useAoMap;

void main() {
    PBRMaterial material;
    material.albedo = (1 - useAlbedoMap) * albedo.rgb + useAlbedoMap * GammaToLinear(texture(albedoMap, varingTexCoords).rgb);
    material.emissive = (1 - useEmissiveMap) * emissive.rgb + useEmissiveMap * GammaToLinear(texture(emissiveMap, varingTexCoords).rgb);
    material.metallic = (1 - useMetallicMap) * metallic + useMetallicMap * texture(metallicMap, varingTexCoords).r;
    material.roughness = (1 - useRoughnessMap) * roughness + useRoughnessMap * texture(roughnessMap, varingTexCoords).r;
    material.ao = (1 - useAoMap) * ao + useAoMap * texture(aoMap, varingTexCoords).r;

    gPosition = fragPos;
    gNormal = normalize(worldNormal);
    gAlbedo.rgb = material.albedo;
    gEmissive.rgb = material.emissive;
    gMetallicRoughnessAO.r = material.metallic;
    gMetallicRoughnessAO.g = material.roughness;
    gMetallicRoughnessAO.b = ao;
    gEnityID.r = varingEntityID;
}