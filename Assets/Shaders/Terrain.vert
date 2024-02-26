#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 varingTexCoords;
out vec3 varingNormal;
out vec3 fragPos;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main() {
    varingNormal = aNormal;
    varingTexCoords = aTexCoords;
    fragPos = vec3(model * vec4(aPos, 1.0));
    fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}