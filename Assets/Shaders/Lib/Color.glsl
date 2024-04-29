#ifndef COLOR_GLSL
#define COLOR_GLSL

vec3 LinearToGamma(vec3 color) {
    return pow(color, vec3(1.0 / 2.2));
}

vec3 GammaToLinear(vec3 color) {
    return pow(color, vec3(2.2));
}

vec3 ToneMapReinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

vec3 ToneMapACES(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

#endif