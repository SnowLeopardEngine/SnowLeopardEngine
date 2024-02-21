#version 460 core

in vec2 varingTexCoords;

out vec4 FragColor;

uniform vec4 baseColor;

void main() {
    FragColor = baseColor;
}