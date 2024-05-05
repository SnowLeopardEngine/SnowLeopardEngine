#version 450

#include "Common/FrameUniform.glsl"

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in int aEntityID;

layout(location = 0) out vec2 varingTexCoords;
layout(location = 1) flat out int varingEntityID;
layout(location = 2) out vec3 worldNormal;
layout(location = 3) out vec3 fragPos;

#ifdef ENABLE_INSTANCING
layout(location = 300) uniform mat4 models[250];
#endif

layout(binding = 0) uniform sampler2D noiseMap;
layout(location = 0) uniform vec3 windDirection;

void main() {
#ifdef ENABLE_INSTANCING
    mat4 model = models[gl_InstanceID];
#else
    mat4 model = getModelMatrix();
#endif
    varingTexCoords = aTexCoords;
    varingEntityID = aEntityID;

    vec4 worldPos = model * vec4(aPos, 1.0);

    float time = getElapsedTime();
    vec2 uv = (worldPos.xz - time) / 30.0;
    float noise = texture(noiseMap, uv).r;
    noise = sin(noise * 0.5);
    float windStrength = 1.0 + noise * 0.3;
    vec3 windEffect = normalize(vec3(windDirection.x, 0.0, windDirection.y)) * windStrength * noise;

    vec3 newPos = worldPos.xyz + windEffect;

    worldNormal = transpose(inverse(mat3(model))) * (aNormal + windEffect);
    fragPos = newPos;

    gl_Position = getProjectionMatrix() * getViewMatrix() * vec4(newPos, 1.0);
}