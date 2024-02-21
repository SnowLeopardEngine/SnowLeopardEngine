#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 varingTexCoords;
out vec3 fragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    Normal = aNormal;
    varingTexCoords = aTexCoords;
    fragPos = aPos * 80;
    gl_Position = projection * view * model * vec4(fragPos, 1.0);
}