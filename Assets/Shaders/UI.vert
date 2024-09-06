#version 450

#include "Common/FrameUniform.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

layout(location = 0) out vec2 varingTexCoords;

void main() {
    varingTexCoords = aTexCoords;
    gl_Position = getOrthoProjectionMatrix() * getModelMatrix() * vec4(aPos.xy, 0.0, 1.0);
}