#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 varingTexCoords;
out vec3 varingNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() 
{
    float wave = sin(aPos.x * 2.0);

    vec3 adjustedPosition = vec3(aPos.x, aPos.y + wave, aPos.z);
    adjustedPosition.z = adjustedPosition.z - 50;
    adjustedPosition.y = adjustedPosition.y - 10;
    adjustedPosition.x = adjustedPosition.x - 50;

    vec3 normalAdjustment = vec3(1, 0, 0);
    vec3 adjustedNormal = normalize(aNormal + normalAdjustment * wave);

    varingNormal = adjustedNormal;
    varingTexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(adjustedPosition, 1.0);
}