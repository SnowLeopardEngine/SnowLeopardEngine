#ifndef COLOR_GLSL
#define COLOR_GLSL

vec3 linearToGamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

vec3 gammaToLinear(vec3 color) {
    return pow(color, vec3(2.2));
}

vec3 toneMapReinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

vec3 toneMapACES(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

float getLuminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

#endif