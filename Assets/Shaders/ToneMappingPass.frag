#version 450

#include "Lib/Color.glsl"

layout(location = 0) in vec2 varyTexCoords;

layout(location = 0) out vec3 FragColor;

layout(location = 0, binding = 0) uniform sampler2D texture0;

void main() {
    const vec4 source = texture(texture0, varyTexCoords);

    vec3 color = LinearToGamma(source.rgb);
    color = ToneMapACES(color);

    FragColor = color;
}