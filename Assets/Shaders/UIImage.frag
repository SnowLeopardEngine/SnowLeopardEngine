#version 450

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec4 FragColor;

layout(location = 0, binding = 0) uniform sampler2D image;
layout(location = 1) uniform int useImage;
layout(location = 2) uniform vec4 baseColor;

void main() {
    FragColor = (1 - useImage) * baseColor + useImage * texture(image, vec2(varingTexCoords.x, 1.0 - varingTexCoords.y)) * baseColor;
}