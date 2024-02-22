#version 460 core

in vec2 varingTexCoords;

out vec4 FragColor;

uniform vec4 baseColor;

void main() {
    FragColor = vec4(0.0, 0.0, 1.0, 1.0);
}