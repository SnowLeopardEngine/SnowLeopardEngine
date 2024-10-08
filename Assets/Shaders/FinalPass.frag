#version 450

layout(location = 0) in vec2 varyTexCoords;

layout(location = 0) out vec4 FragColor;

layout(location = 0, binding = 0) uniform sampler2D texture0;

void main() {
    const vec4 target = texture(texture0, varyTexCoords);
    FragColor = vec4(target.rgb, 1.0);
}