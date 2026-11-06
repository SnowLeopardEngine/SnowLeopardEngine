#version 450

#include "Lib/Color.glsl"

layout(location = 0) in vec2 varyTexCoords;

layout(location = 0) out vec3 FragColor;

layout(location = 0, binding = 0) uniform sampler2D texture0;

void main() {
    const vec4 source = texture(texture0, varyTexCoords);

    // Tone mapping works on linear HDR radiance; gamma encoding for the display comes last.
    // toneMapACES is Narkowicz's fit of the ACES RRT+ODT, which expects its input pre-scaled by 0.6 to match the
    // reference curve. TODO: make exposure a camera / render setting.
    const float exposure = 0.6;
    vec3 color = toneMapACES(source.rgb * exposure);
    color = linearToGamma(color);

    FragColor = color;
}