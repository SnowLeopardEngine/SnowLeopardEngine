#version 450

#include "Lib/Math.glsl"
#include "Lib/Color.glsl"
#include "Lib/Texture.glsl"

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec3 FragColor;

layout(location = 0, binding = 0) uniform sampler2D texture0;
layout(location = 1) uniform int mipLevel;

float karisAverage(vec3 color) {
    const float luma = getLuminance(linearToGamma(color)) * 0.25;
    return 1.0 / (1.0 + luma);
}

void main() {
    vec2 srcTexelSize = getTexelSize(texture0);
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current (e) texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    vec3 a = texture(texture0, vec2(varingTexCoords.x - 2.0 * x, varingTexCoords.y + 2.0 * y)).rgb;
    vec3 b = texture(texture0, vec2(varingTexCoords.x, varingTexCoords.y + 2.0 * y)).rgb;
    vec3 c = texture(texture0, vec2(varingTexCoords.x + 2.0 * x, varingTexCoords.y + 2.0 * y)).rgb;

    vec3 d = texture(texture0, vec2(varingTexCoords.x - 2.0 * x, varingTexCoords.y)).rgb;
    vec3 e = texture(texture0, vec2(varingTexCoords.x, varingTexCoords.y)).rgb;
    vec3 f = texture(texture0, vec2(varingTexCoords.x + 2.0 * x, varingTexCoords.y)).rgb;

    vec3 g = texture(texture0, vec2(varingTexCoords.x - 2.0 * x, varingTexCoords.y - 2.0 * y)).rgb;
    vec3 h = texture(texture0, vec2(varingTexCoords.x, varingTexCoords.y - 2.0 * y)).rgb;
    vec3 i = texture(texture0, vec2(varingTexCoords.x + 2.0 * x, varingTexCoords.y - 2.0 * y)).rgb;

    vec3 j = texture(texture0, vec2(varingTexCoords.x - x, varingTexCoords.y + y)).rgb;
    vec3 k = texture(texture0, vec2(varingTexCoords.x + x, varingTexCoords.y + y)).rgb;
    vec3 l = texture(texture0, vec2(varingTexCoords.x - x, varingTexCoords.y - y)).rgb;
    vec3 m = texture(texture0, vec2(varingTexCoords.x + x, varingTexCoords.y - y)).rgb;

    vec3 groups[5];
    if(mipLevel == 0) {
        groups[0] = (a + b + d + e) * (0.125 / 4.0);
        groups[1] = (b + c + e + f) * (0.125 / 4.0);
        groups[2] = (d + e + g + h) * (0.125 / 4.0);
        groups[3] = (e + f + h + i) * (0.125 / 4.0);
        groups[4] = (j + k + l + m) * (0.5 / 4.0);
        groups[0] *= karisAverage(groups[0]);
        groups[1] *= karisAverage(groups[1]);
        groups[2] *= karisAverage(groups[2]);
        groups[3] *= karisAverage(groups[3]);
        groups[4] *= karisAverage(groups[4]);
        FragColor = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
        FragColor = max(FragColor, EPSILON);
    } else {
        FragColor = e * 0.125;
        FragColor += (a + c + g + i) * 0.03125;
        FragColor += (b + d + f + h) * 0.0625;
        FragColor += (j + k + l + m) * 0.125;
    }
}