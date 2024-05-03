#ifndef CASCADED_UNIFORM_GLSL
#define CASCADED_UNIFORM_GLSL

#define MAX_NUM_CASCADES 4

layout(binding = 2, std140) uniform Cascades {
    vec4 splitDepth;
    mat4 lightSpaceMatrices[MAX_NUM_CASCADES];
} uCascades;

uint selectCascadeIndex(vec3 fragPosViewSpace) {
    uint cascadeIndex = 0;
    for(uint i = 0; i < MAX_NUM_CASCADES - 1; ++i) {
        if(fragPosViewSpace.z < uCascades.splitDepth[i]) {
            cascadeIndex = i + 1;
        }
    }
    return cascadeIndex;
}

#endif