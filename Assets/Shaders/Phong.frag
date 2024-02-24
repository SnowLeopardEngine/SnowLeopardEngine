#version 460 core

in vec2 varingTexCoords;
in vec3 varingNormal;
in vec3 fragPos;

out vec4 FragColor;

uniform vec4 baseColor;
uniform vec3 viewPos;

struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
};
uniform DirectionalLight directionalLight;

uniform int useDiffuse;
uniform sampler2D diffuseMap;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuseColor = (useDiffuse * texture(diffuseMap, varingTexCoords).rgb + (1 - useDiffuse) * baseColor.rgb);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.7);

    vec3 ambient = 0.5 * light.intensity * diffuseColor;
    vec3 diffuse = light.intensity * light.color * diff * diffuseColor;
    vec3 specular = light.intensity * light.color * spec * vec3(0.1, 0.1, 0.1);

    return ambient + diffuse + specular;
}

void main() {
    // for now, only care about the directional light
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 finalColor = CalculateDirectionalLight(directionalLight, varingNormal, viewDir);

    FragColor = vec4(finalColor, 1);
}
