#version 450

#include "Common/FrameUniform.glsl"

layout(location = 0) in vec3 aPos;

void main() {
    vec4 pos = vec4(aPos, 1.0);
    gl_Position = getLightSpaceMatrix() * getModelMatrix() * pos;
}