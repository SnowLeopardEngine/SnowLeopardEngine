#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in int aEntityID;
layout(location = 4) in ivec4 aBoneIDs;
layout(location = 5) in vec4 aWeights;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

out vec2 varingTexCoords;
out vec3 varingNormal;
out int varingEntityID;
out vec3 fragPos;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;
uniform int hasAnimation;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {
    varingNormal = aNormal;
    varingTexCoords = aTexCoords;
    varingEntityID = aEntityID;
    fragPos = vec3(model * vec4(aPos, 1.0));
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);

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
    gl_Position = projection * view * model * pos;
}