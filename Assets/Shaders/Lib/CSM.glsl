#ifndef CSM_GLSL_
#define CSM_GLSL_

#include <Common/CascadedUniform.glsl>

float calShadow(uint cascadeIndex, vec3 fragPos, vec3 normal, vec3 lightDir, sampler2DArrayShadow cascadedShadowMaps) {
    mat4 lightSpaceMatrix = uCascades.lightSpaceMatrices[cascadeIndex];
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if(projCoords.z > 1.0)
        return 0.0;

    float depthOffset = 0.001;
    vec4 shadowCoord = lightSpaceMatrix * vec4(fragPos + normal * depthOffset, 1.0);

    float bias = 0.005;

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(cascadedShadowMaps, 0).xy;

    for(float x = -1.5; x <= 1.5; ++x) {
        for(float y = -1.5; y <= 1.5; ++y) {
            shadow += (1.0 - texture(cascadedShadowMaps, vec4(shadowCoord.xy + vec2(x, y) * texelSize, cascadeIndex, shadowCoord.z - bias)).r);
        }
    }

    return clamp(shadow / 9.0, 0.0, 1.0);
}

#endif
