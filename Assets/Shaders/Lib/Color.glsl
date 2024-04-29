#ifndef COLOR_GLSL
#define COLOR_GLSL

vec3 CorrectGamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

// reinhard
vec3 ToneMap(vec3 color) {
    return color / (color + vec3(1.0));
}

#endif