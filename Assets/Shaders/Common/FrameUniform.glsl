#ifndef FRAME_UNIFORM_GLSL
#define FRAME_UNIFORM_GLSL

layout(binding = 0, std140) uniform FrameUniform {
    float elapsedTime;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 viewPos;
} uFrame;

layout(location = 100) uniform mat4 model;

mat4 getModelMatrix() {
    return model;
}

mat4 getViewMatrix() {
    return uFrame.viewMatrix;
}

mat4 getProjectionMatrix() {
    return uFrame.projectionMatrix;
}

vec3 getViewPos() {
    return uFrame.viewPos;
}

float getElapsedTime() {
    return uFrame.elapsedTime;
}

#endif