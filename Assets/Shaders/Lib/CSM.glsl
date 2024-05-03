#ifndef CSM_GLSL_
#define CSM_GLSL_

#include <Common/CascadedUniform.glsl>

float calShadow(uint cascadeIndex, vec3 fragPos, sampler2DArrayShadow cascadedShadowMaps) {
    vec4 shadowCoord = uCascades.lightSpaceMatrices[cascadeIndex] * vec4(fragPos, 1.0);

    return 1.0 - texture(cascadedShadowMaps, vec4(shadowCoord.xy, cascadeIndex, shadowCoord.z - 0.01));
}

#endif