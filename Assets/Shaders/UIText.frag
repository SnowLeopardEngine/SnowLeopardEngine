#version 450

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec4 FragColor;

layout(location = 0, binding = 0) uniform sampler2D text;
layout(location = 1) uniform vec4 baseColor;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, varingTexCoords).r);
    FragColor = vec4(baseColor.rgb, 1.0) * sampled;
}