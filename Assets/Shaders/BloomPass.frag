#version 450

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec4 FragColor;

layout(location = 0, binding = 0) uniform sampler2D sceneColor;
layout(location = 1, binding = 1) uniform sampler2D blur;

layout(location = 2) uniform float strength;

void main() {
    const vec3 sceneColor = texture(sceneColor, varingTexCoords).rgb;
    const vec3 blurred = texture(blur, varingTexCoords).rgb;
    FragColor.rgb = mix(sceneColor, blurred, strength);
}