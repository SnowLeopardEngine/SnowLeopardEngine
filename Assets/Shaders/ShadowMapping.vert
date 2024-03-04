#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 4) in ivec4 aBoneIDs;
layout(location = 5) in vec4 aWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform int hasAnimation;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {
    // animation part
    vec4 totalPosition = vec4(0.0);
    for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        if(aBoneIDs[i] == -1)
            continue;
        if(aBoneIDs[i] >= MAX_BONES) {
            totalPosition = vec4(aPos, 1.0);
            break;
        }
        vec4 localPosition = finalBonesMatrices[aBoneIDs[i]] * vec4(aPos, 1.0);
        totalPosition += localPosition * aWeights[i];
    }

    vec4 pos = hasAnimation * totalPosition + (1 - hasAnimation) * vec4(aPos, 1.0);

    gl_Position = lightSpaceMatrix * model * pos;
}