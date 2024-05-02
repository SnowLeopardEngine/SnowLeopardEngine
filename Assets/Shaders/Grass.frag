#version 450

#include "Common/FrameUniform.glsl"
#include "Common/LightUniform.glsl"
#include "Lib/PBRLighting.glsl"
#include "Lib/Color.glsl"

layout(location = 0) in vec2 varingTexCoords;
layout(location = 2) in vec3 worldNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 accum;
layout(location = 1) out float reveal;

layout(location = 0, binding = 0) uniform sampler2D albedoMap;
layout(location = 1, binding = 1) uniform sampler2D emissiveMap;
layout(location = 2, binding = 2) uniform sampler2D metallicMap;
layout(location = 3, binding = 3) uniform sampler2D roughnessMap;
layout(location = 4, binding = 4) uniform sampler2D aoMap;
layout(location = 5, binding = 5) uniform sampler2D shadowMap;
layout(location = 6, binding = 6) uniform sampler2D brdfLUT;
layout(location = 7, binding = 7) uniform samplerCube irradianceMap;
layout(location = 8, binding = 8) uniform samplerCube prefilteredEnvMap;

layout(location = 9) uniform vec4 albedo;
layout(location = 10) uniform int useAlbedoMap;
layout(location = 11) uniform vec4 emissive;
layout(location = 12) uniform int useEmissiveMap;
layout(location = 13) uniform float metallic;
layout(location = 14) uniform int useMetallicMap;
layout(location = 15) uniform float roughness;
layout(location = 16) uniform int useRoughnessMap;
layout(location = 17) uniform float ao;
layout(location = 18) uniform int useAoMap;
layout(location = 19) uniform float opacity;

void main() {
    PBRMaterial material;
    material.albedo = (1 - useAlbedoMap) * albedo.rgb + useAlbedoMap * gammaToLinear(texture(albedoMap, varingTexCoords).rgb);
    // material.emissive = (1 - useEmissiveMap) * emissive.rgb + useEmissiveMap * gammaToLinear(texture(emissiveMap, varingTexCoords).rgb);
    // material.metallic = (1 - useMetallicMap) * metallic + useMetallicMap * texture(metallicMap, varingTexCoords).r;
    // material.roughness = (1 - useRoughnessMap) * roughness + useRoughnessMap * texture(roughnessMap, varingTexCoords).r;
    // material.ao = (1 - useAoMap) * ao + useAoMap * texture(aoMap, varingTexCoords).r;
    material.opacity = opacity;

    // vec3 viewDir = normalize(getViewPos() - fragPos);
    // vec3 color = calPBRLighting(getDirectionalLight(), getPointLights(), getNumPointLights(), worldNormal, viewDir, material, fragPos, shadowMap, brdfLUT, irradianceMap, prefilteredEnvMap);

    const float a = clamp(material.opacity, 0.0, 1.0);

    // const float w = clamp(pow(min(1.0, a * 10.0) + 0.01, 3.0) * 1e8 *
    //     pow(1.0 - gl_FragCoord.z * 0.9, 3.0), 1e-2, 3e3);

    // color = toneMapACES(color);
    // accum = vec4(color * a, a) * w;
    accum = vec4(gammaToLinear(texture(albedoMap, varingTexCoords).rgb), 1);
    reveal = a;
}