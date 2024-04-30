#version 450

// https://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/

#include "Lib/Texture.glsl"

const float kOffsets[3] = { 0.0, 1.3846153846, 3.2307692308 };
const float kWeights[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec3 FragColor;

layout(binding = 0) uniform sampler2D texture0;

layout(location = 0) uniform bool horizontal;
layout(location = 1) uniform float scale = 1.0;

vec3 HorizontalBlur() {
    const float texOffset = getTexelSize(texture0).x * scale;
    vec3 result = texture(texture0, varingTexCoords).rgb * kWeights[0];
    for(uint i = 1; i < 3; ++i) {
        result += texture(texture0, varingTexCoords + vec2(texOffset * i, 0.0)).rgb * kWeights[i];
        result += texture(texture0, varingTexCoords - vec2(texOffset * i, 0.0)).rgb * kWeights[i];
    }
    return result;
}

vec3 VerticalBlur() {
    const float texOffset = getTexelSize(texture0).y * scale;
    vec3 result = texture(texture0, varingTexCoords).rgb * kWeights[0];
    for(uint i = 1; i < 3; ++i) {
        result += texture(texture0, varingTexCoords + vec2(0.0, texOffset * i)).rgb * kWeights[i];
        result += texture(texture0, varingTexCoords - vec2(0.0, texOffset * i)).rgb * kWeights[i];
    }
    return result;
}

void main() {
    FragColor = horizontal ? HorizontalBlur() : VerticalBlur();
}