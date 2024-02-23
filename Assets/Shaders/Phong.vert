#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 varingTexCoords;
out vec3 varingNormal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    varingNormal = aNormal;
    varingTexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    fragPos = gl_Position.xyz;
}