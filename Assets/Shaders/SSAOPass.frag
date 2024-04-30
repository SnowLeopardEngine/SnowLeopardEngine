#version 450

#include "Common/FrameUniform.glsl"
#include "Lib/Depth.glsl"

layout(location = 0) in vec2 varingTexCoords;

layout(binding = 0) uniform sampler2D gDepth;
layout(binding = 1) uniform sampler2D gNormal;
layout(binding = 2) uniform sampler2D noiseMap;

layout(binding = 1, std140) uniform KernelBlock {
    vec4 samples[KERNEL_SIZE];
} uKernel;

const float kRadius = 0.5;
const float kBias = 0.025;

layout(location = 0) out float FragColor;
void main() {
    const float depth = texture(gDepth, varingTexCoords).r;
    if(depth >= 1.0)
        discard;

    const vec2 gBufferSize = textureSize(gDepth, 0);
    const vec2 noiseSize = textureSize(noiseMap, 0);
    const vec2 noiseTexCoord = (gBufferSize / noiseSize) * varingTexCoords;
    const vec3 rvec = texture(noiseMap, noiseTexCoord).xyz;

    vec3 N = normalize(texture(gNormal, varingTexCoords).rgb);
    N = mat3(getViewMatrix()) * N;
    const vec3 T = normalize(rvec - N * dot(rvec, N));
    const vec3 B = cross(N, T);
    const mat3 TBN = mat3(T, B, N);

    const vec3 fragPosViewSpace = viewPositionFromDepth(depth, varingTexCoords);

    float occlusion = 0.0;
    for(uint i = 0; i < KERNEL_SIZE; ++i) {
        vec3 samplePos = TBN * uKernel.samples[i].xyz;
        samplePos = fragPosViewSpace + samplePos * kRadius;
        const vec2 offset = viewToClip(vec4(samplePos, 1.0)).xy * 0.5 + 0.5;
        float sampleDepth = texture(gDepth, offset).r;
        sampleDepth = viewPositionFromDepth(sampleDepth, offset).z;

        const float rangeCheck = smoothstep(0.0, 1.0, kRadius / abs(fragPosViewSpace.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + kBias ? 1.0 : 0.0) * rangeCheck;
    }

    FragColor = 1.0 - occlusion / KERNEL_SIZE;
}