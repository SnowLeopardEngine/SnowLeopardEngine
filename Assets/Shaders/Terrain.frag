#version 460 core

in vec2 varingTexCoords;
in vec3 varingNormal;

out vec4 FragColor;

uniform vec4 baseColor;
uniform int useDiffuse;
uniform sampler2D diffuseMap;

void main() {
    vec3 diffuseColor = texture2D(diffuseMap, varingTexCoords).xyz;
    vec3 finalDiffuseColor = diffuseColor * useDiffuse + baseColor.xyz * (1 - useDiffuse);

    FragColor = vec4(finalDiffuseColor, 1.0);
}