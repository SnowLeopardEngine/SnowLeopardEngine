#version 450

#include "Lib/Math.glsl"

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D accum;
layout(binding = 1) uniform sampler2D reveal;

void main() {
    const ivec2 coords = ivec2(gl_FragCoord.xy);
    const float revealage = texelFetch(reveal, coords, 0).r;

    // Save the blending and color texture fetch cost if there is not a
    // transparent fragment
    if(isApproximatelyEqual(revealage, 1.0)) {
        discard;
        return;
    }

    vec4 accumulation = texelFetch(accum, coords, 0);

    // Suppress overflow
    if(isinf(max3(abs(accumulation.rgb))))
        accumulation.rgb = vec3(accumulation.a);

    const vec3 averageColor = accumulation.rgb / max(accumulation.a, EPSILON);
    FragColor = vec4(averageColor, 1.0 - revealage);
}