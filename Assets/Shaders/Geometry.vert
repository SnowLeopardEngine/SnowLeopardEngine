#version 450

#include "Common/FrameUniform.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in int aEntityID;

layout(location = 0) out vec2 varingTexCoords;
layout(location = 1) flat out int varingEntityID;
layout(location = 2) out vec3 worldNormal;
layout(location = 3) out vec3 fragPos;

void main() {
    varingTexCoords = aTexCoords;
    varingEntityID = aEntityID;
    worldNormal = transpose(inverse(mat3(model))) * aNormal;
    vec4 worldPos = getModelMatrix() * vec4(aPos, 1.0);
    fragPos = worldPos.xyz;
    gl_Position = getProjectionMatrix() * getViewMatrix() * worldPos;
}