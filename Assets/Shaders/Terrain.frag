#version 460 core

in vec2 varingTexCoords;
in vec3 varingNormal;

out vec4 FragColor;

uniform vec4 baseColor;
uniform sampler2D diffuseMap;

void main() 
{
    vec3 diffuse = texture2D(diffuseMap, varingTexCoords).xyz;

    FragColor = vec4(varingNormal, 1.0);
}