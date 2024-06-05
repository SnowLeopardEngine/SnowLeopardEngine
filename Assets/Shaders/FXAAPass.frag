#version 450

// https://www.shadertoy.com/view/ls3GWS
// https://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/3/
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf

#include "Common/FrameUniform.glsl"

layout(location = 0) out vec3 FragColor;

layout(location = 0, binding = 0) uniform sampler2D texture0;

#define FXAA_SPAN_MAX 8.0
#define FXAA_REDUCE_MUL (1.0 / FXAA_SPAN_MAX)
#define FXAA_REDUCE_MIN (1.0 / 128.0)
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)

void main() {
    const vec2 rcpFrame = getTexelSize();
    const vec2 uv2 = gl_FragCoord.xy / getResolution();
    const vec4 uv = vec4(uv2, uv2 - (rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT)));

    const vec3 rgbNW = textureLod(texture0, uv.zw, 0.0).xyz;
    const vec3 rgbNE = textureLod(texture0, uv.zw + vec2(1.0, 0.0) * rcpFrame.xy, 0.0).xyz;
    const vec3 rgbSW = textureLod(texture0, uv.zw + vec2(0.0, 1.0) * rcpFrame.xy, 0.0).xyz;
    const vec3 rgbSE = textureLod(texture0, uv.zw + vec2(1.0, 1.0) * rcpFrame.xy, 0.0).xyz;
    const vec3 rgbM = textureLod(texture0, uv.xy, 0.0).xyz;

    const vec3 luma = vec3(0.299, 0.587, 0.114);
    const float lumaNW = dot(rgbNW, luma);
    const float lumaNE = dot(rgbNE, luma);
    const float lumaSW = dot(rgbSW, luma);
    const float lumaSE = dot(rgbSE, luma);
    const float lumaM = dot(rgbM, luma);

    const float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    const float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    const float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    const float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcpDirMin)) *
        rcpFrame.xy;

    const vec3 rgbA = (1.0 / 2.0) * (textureLod(texture0, uv.xy + dir * (1.0 / 3.0 - 0.5), 0.0).xyz +
        textureLod(texture0, uv.xy + dir * (2.0 / 3.0 - 0.5), 0.0).xyz);
    const vec3 rgbB = rgbA * (1.0 / 2.0) +
        (1.0 / 4.0) * (textureLod(texture0, uv.xy + dir * (0.0 / 3.0 - 0.5), 0.0).xyz +
        textureLod(texture0, uv.xy + dir * (3.0 / 3.0 - 0.5), 0.0).xyz);

    const float lumaB = dot(rgbB, luma);
    FragColor = (lumaB < lumaMin) || (lumaB > lumaMax) ? rgbA : rgbB;
}