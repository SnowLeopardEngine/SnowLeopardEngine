#ifndef DEPTH_GLSL
#define DEPTH_GLSL

vec3 viewPositionFromDepth(float z, vec2 texCoord) {
  // https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy/46118945#46118945
    return clipToView(vec4(texCoord * 2.0 - 1.0, z, 1.0));
}

#endif