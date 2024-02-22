#version 460 core

in vec2 varingTexCoords;
in vec3 varingNormal;
in vec3 fragPos;

out vec4 FragColor;

uniform vec4 baseColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform int useDiffuse;
uniform sampler2D diffuseMap;

void main() {
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 ambient = 0.5 * vec3(baseColor.x, baseColor.y, baseColor.z);

    float diff = max(dot(varingNormal, lightDir), 0.0);
    vec3 diffuse = diff * (texture2D(diffuseMap, varingTexCoords).xyz * useDiffuse + baseColor.xyz * (1 - useDiffuse));

    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, varingNormal);
    vec3 specularColor = vec3(0.1, 0.1, 0.1);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.7);
    vec3 specular = specularColor * spec;

    vec3 finalColor = ambient + diffuse + specular;

    FragColor = vec4(finalColor, 1);
}
