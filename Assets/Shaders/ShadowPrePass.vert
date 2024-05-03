#version 450

#include "Common/FrameUniform.glsl"

layout(location = 0) in vec3 aPos;

layout(location = 0) uniform mat4 shadowLightViewProjection;

#ifdef ENABLE_INSTANCING
layout(location = 300) uniform mat4 models[250];
#endif

void main() {
#ifdef ENABLE_INSTANCING
    mat4 model = models[gl_InstanceID];
#else
    mat4 model = getModelMatrix();
#endif
    gl_Position = shadowLightViewProjection * model * vec4(aPos, 1.0);
}