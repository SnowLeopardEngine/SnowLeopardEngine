#version 450

layout(location = 0) in vec2 varingTexCoords;

layout(location = 0) out vec3 FragColor;

layout(location = 0, binding = 0) uniform sampler2D texture0;
layout(location = 1) uniform float radius;

void main() {
    float x = radius;
    float y = radius;

    // Take 9 samples around current (e) texel:
    // a - b - c
    // d - e - f
    // g - h - i
    vec3 a = texture(texture0, vec2(varingTexCoords.x - x, varingTexCoords.y + y)).rgb;
    vec3 b = texture(texture0, vec2(varingTexCoords.x, varingTexCoords.y + y)).rgb;
    vec3 c = texture(texture0, vec2(varingTexCoords.x + x, varingTexCoords.y + y)).rgb;

    vec3 d = texture(texture0, vec2(varingTexCoords.x - x, varingTexCoords.y)).rgb;
    vec3 e = texture(texture0, vec2(varingTexCoords.x, varingTexCoords.y)).rgb;
    vec3 f = texture(texture0, vec2(varingTexCoords.x + x, varingTexCoords.y)).rgb;

    vec3 g = texture(texture0, vec2(varingTexCoords.x - x, varingTexCoords.y - y)).rgb;
    vec3 h = texture(texture0, vec2(varingTexCoords.x, varingTexCoords.y - y)).rgb;
    vec3 i = texture(texture0, vec2(varingTexCoords.x + x, varingTexCoords.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    FragColor = e * 4.0;
    FragColor += (b + d + f + h) * 2.0;
    FragColor += (a + c + g + i);
    FragColor *= 1.0 / 16.0;
}