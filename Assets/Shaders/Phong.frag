#version 460 core

in vec2 varingTexCoords;
in vec3 varingNormal;

out vec4 FragColor;

uniform vec4 baseColor;

void main() {
    vec3 lightDir = normalize(vec3(-0.6, -0.8, -0.5));
    vec3 ambient = 0.5 * vec3(baseColor.x, baseColor.y, baseColor.z);

    float diff = max(dot(varingNormal, -(lightDir)), 0.0);
    vec3 diffuse = diff * baseColor.xyz;

    vec3 viewDir = vec3(0, 0, 0);
    vec3 reflectDir = normalize(lightDir - 2 * dot(varingNormal, lightDir) * varingNormal);
    vec3 specularColor = vec3(0.1, 0.1, 0.1);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.7);
    vec3 specular = specularColor * spec;

    vec3 finalColor = ambient + diffuse + specular;

    FragColor = vec4(finalColor, 1);
}