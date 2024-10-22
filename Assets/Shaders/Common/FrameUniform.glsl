#ifndef FRAME_UNIFORM_GLSL
#define FRAME_UNIFORM_GLSL

layout(binding = 0, std140) uniform FrameUniform {
    float elapsedTime;
    mat4 viewMatrix;
    mat4 inversedViewMatrix;
    mat4 projectionMatrix;
    mat4 inversedProjectionMatrix;
    mat4 orthoProjectionMatrix;
    vec3 viewPos;
    vec2 resolution;
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

mat4 getOrthoProjectionMatrix() {
    return uFrame.orthoProjectionMatrix;
}

vec3 getViewPos() {
    return uFrame.viewPos;
}

float getElapsedTime() {
    return uFrame.elapsedTime;
}

vec4 worldToView(vec4 v) {
    return uFrame.viewMatrix * v;
}

vec4 viewToWorld(vec4 v) {
    return uFrame.inversedViewMatrix * v;
}

vec3 viewToClip(vec4 v) {
    vec4 clip = uFrame.projectionMatrix * v;
    return clip.xyz / clip.w;
}

vec3 clipToView(vec4 v) {
    const vec4 view = uFrame.inversedProjectionMatrix * v;
    return view.xyz / view.w;
}

vec2 getResolution() {
    return uFrame.resolution;
}

vec2 getTexelSize() {
    return 1.0 / getResolution();
}

#endif