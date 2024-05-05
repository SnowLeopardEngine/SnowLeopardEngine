#ifndef LIGHT_UNIFORM_GLSL
#define LIGHT_UNIFORM_GLSL

#include "Common/Light.glsl"

layout(binding = 1, std140) uniform LightUniform {
    uint numPointLights;
    DirectionalLight directionalLight;
    PointLight pointLights[NUM_MAX_POINT_LIGHT];
} uLight;

DirectionalLight getDirectionalLight() {
    return uLight.directionalLight;
}

uint getNumPointLights() {
    return uLight.numPointLights;
}

PointLight[NUM_MAX_POINT_LIGHT] getPointLights() {
    return uLight.pointLights;
}

PointLight getPointLight(uint index) {
    return uLight.pointLights[index];
}

#endif