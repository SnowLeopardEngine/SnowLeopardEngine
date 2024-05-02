#version 450

#include "Common/FrameUniform.glsl"

layout(location = 0) out vec3 eyeDirection;

void main() {
    vec2 texCoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(texCoord * 2.0 - 1.0, 1.0, 1.0);
    eyeDirection = vec3(viewToWorld(vec4(clipToView(gl_Position), 0.0)));
    gl_Position.z = gl_Position.w * 0.999999;
}