#include "LegacySceneUniform.glsl"

#ifdef FORWARD_LIGHTING
struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
};
uniform DirectionalLight directionalLight;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseColor) {
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.7);

    vec3 ambient = 0.5 * light.intensity * diffuseColor;
    vec3 diffuse = light.intensity * light.color * diff * diffuseColor;
    vec3 specular = light.intensity * light.color * spec * vec3(0.1, 0.1, 0.1);

    return ambient + diffuse + specular;
}
#endif