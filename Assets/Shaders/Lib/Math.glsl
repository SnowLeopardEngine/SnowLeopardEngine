#ifndef MATH_GLSL
#define MATH_GLSL

#define PI 3.1415926535897932384626433832795
#define TWO_PI 6.2831853071795864769252867665590
#define HALF_PI 1.5707963267948966192313216916398
#define EPSILON 0.00001

float random(vec2 co) {
    const float a = 12.9898;
    const float b = 78.233;
    const float c = 43758.5453;
    const float dt = dot(co, vec2(a, b));
    const float sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

mat3 generateTBN(vec3 N) {
    vec3 B = vec3(0.0, 1.0, 0.0);
    float NdotUp = dot(N, vec3(0.0, 1.0, 0.0));
    if(1.0 - abs(NdotUp) <= EPSILON) {
        B = (NdotUp > 0.0) ? vec3(0.0, 0.0, 1.0) : vec3(0.0, 0.0, -1.0);
    }
    vec3 T = normalize(cross(B, N));
    B = cross(N, T);
    return mat3(T, B, N);
}

float _radicalInverse_VdC(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley2D(uint i, uint N) {
    return vec2(float(i) / float(N), _radicalInverse_VdC(i));
}

vec3 importanceSampleGGX(vec2 Xi, float roughness, vec3 N) {
    const float a = roughness * roughness;
    const float phi = 2.0 * PI * Xi.x + random(N.xz) * 0.1;
    const float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    const float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    // From spherical coordinates to cartesian coordinates
    const vec3 H = vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    // From tangent-space H vector to world-space sample vector
    const vec3 up = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    const vec3 tangent = normalize(cross(up, N));
    const vec3 bitangent = cross(N, tangent);
    // Tangent to world space
    return tangent * H.x + bitangent * H.y + N * H.z;
}

#endif