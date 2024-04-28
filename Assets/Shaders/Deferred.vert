#version 450

#include "LegacySceneUniform.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

layout(location = 0) out vec2 varingTexCoords;

void main() {
    varingTexCoords = aTexCoords;
    gl_Position = vec4(aPos, 1.0);
}