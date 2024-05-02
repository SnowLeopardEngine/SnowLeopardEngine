#version 450

layout(location = 0) in vec3 eyeDirection;

layout(location = 0) out vec3 FragColor;

layout(location = 0, binding = 0) uniform samplerCube skyBox;

void main() {
    FragColor = texture(skyBox, eyeDirection).rgb;
}