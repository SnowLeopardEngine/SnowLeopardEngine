#version 460 core

in vec3 cubeMapTextureCoords;

layout(location = 0) out vec4 color0;
layout(location = 1) out int color1;

uniform samplerCube cubeMap;

void main() {
    color0 = texture(cubeMap, cubeMapTextureCoords);
    color1 = -1;
}