#version 460 core

in vec2 varingTexCoords;
in vec3 normal;
in vec3 fragPos;
in vec4 fragPosLightSpace;

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

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // remap to [0, 1] to fit depth

    if(projCoords.z > 1.0)
        return 0.0;

    float currentDepth = projCoords.z;

    // apply PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuseColor = (useDiffuse * texture(diffuseMap, varingTexCoords).rgb + (1 - useDiffuse) * baseColor.rgb);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.7);

    vec3 ambient = 0.5 * light.intensity * diffuseColor;
    vec3 diffuse = light.intensity * light.color * diff * diffuseColor;
    vec3 specular = light.intensity * light.color * spec * vec3(0.1, 0.1, 0.1);

    float shadow = ShadowCalculation(fragPosLightSpace);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

void main() {
    // for now, only care about the directional light
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 finalColor = CalculateDirectionalLight(directionalLight, normal, viewDir);

    FragColor = vec4(finalColor, 1);
}
