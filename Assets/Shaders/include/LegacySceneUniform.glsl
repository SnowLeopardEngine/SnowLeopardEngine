const int MAX_BONES = 128;
const int MAX_BONE_INFLUENCE = 4;

layout(location = 100) uniform mat4 model;
layout(location = 101) uniform mat4 view;
layout(location = 102) uniform mat4 projection;
layout(location = 103) uniform vec3 viewPos;
layout(location = 104) uniform int hasAnimation;
layout(location = 105) uniform mat4 finalBonesMatrices[MAX_BONES];