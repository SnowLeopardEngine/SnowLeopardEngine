uniform UScene {
    mat4 model;
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
} uScene;

#ifdef DEFERRED_LIGHTING
vec3 CalculateLighting() { return vec3(0, 0, 0); }
#endif