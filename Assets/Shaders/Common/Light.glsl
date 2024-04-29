#ifndef LIGHT_GLSL
#define LIGHT_GLSL

#define NUM_MAX_POINT_LIGHT 4

struct DirectionalLight {
    float intensity;
    mat4 lightSpaceMatrix;
    vec3 direction;
    vec3 color;
};

struct PointLight {
    float constant;
    float linear;
    float quadratic;
    float intensity;
    mat4 lightSpaceMatrix;
    vec3 position;
    vec3 color;
};

#endif