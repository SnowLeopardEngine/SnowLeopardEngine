#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec2 varingTexCoords;
out vec3 varingNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int useDiffuse;
uniform sampler2D diffuseMap;

float getHeightValue(vec2 uv) 
{
    vec3 heightData = texture2D(diffuseMap, aTexCoords).xyz;
    return heightData.x * 65536.0 + heightData.y * 256.0 + heightData.z;
}

void main() 
{
    vec2 texelSize = 25.0 / textureSize(diffuseMap, 0);
    float heightValue = getHeightValue(aTexCoords);

    heightValue = 0.0003 * heightValue;

    vec3 adjustedPosition = vec3(aPos.x, aPos.y + heightValue, aPos.z);
    adjustedPosition.z = adjustedPosition.z - 50;
    adjustedPosition.y = adjustedPosition.y - 50;
    adjustedPosition.x = adjustedPosition.x - 50;

    vec3 normalAdjustment = vec3(1, 0, 0);
    vec3 adjustedNormal = normalize(aNormal + normalAdjustment);

    varingNormal = adjustedNormal;
    varingTexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(adjustedPosition, 1.0);
}