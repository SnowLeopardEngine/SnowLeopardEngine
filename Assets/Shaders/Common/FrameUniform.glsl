#ifndef FRAME_UNIFORM_GLSL
#define FRAME_UNIFORM_GLSL

layout(binding = 0, std140) uniform FrameUniform {
    float DirectionalLightIntensity;
    float ElapsedTime;

    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 LightSpaceMatrix;

    vec3 ViewPos;
    vec3 DirectionalLightDirection;
    vec3 DirectionalLightColor;
} uFrameUniform;

layout(location = 100) uniform mat4 model;

mat4 getModelMatrix() {
    return model;
}

mat4 getViewMatrix() {
    return uFrameUniform.ViewMatrix;
}

mat4 getProjectionMatrix() {
    return uFrameUniform.ProjectionMatrix;
}

mat4 getLightSpaceMatrix() {
    return uFrameUniform.LightSpaceMatrix;
}

vec3 getViewPos() {
    return uFrameUniform.ViewPos;
}

vec3 getDirectionalLightDirection() {
    return uFrameUniform.DirectionalLightDirection;
}

vec3 getDirectionalLightColor() {
    return uFrameUniform.DirectionalLightColor;
}

float getDirectionalLightIntensity() {
    return uFrameUniform.DirectionalLightIntensity;
}

float getElapsedTime() {
    return uFrameUniform.ElapsedTime;
}

#endif