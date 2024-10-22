#version 450

layout(location = 0) in vec2 varyTexCoords;

layout(location = 0) out vec4 FragColor;

layout(location = 0, binding = 0) uniform sampler2D source;
layout(location = 1, binding = 1) uniform sampler2D target;

void main() {
    const vec4 sourceSampled = texture(source, varyTexCoords);
    const vec4 targetSampled = vec4(texture(target, varyTexCoords).rgb, 1.0);

    vec4 blendedColor = sourceSampled * sourceSampled.a + targetSampled * (1.0 - sourceSampled.a);

    FragColor = blendedColor;
}