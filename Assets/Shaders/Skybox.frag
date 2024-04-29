#version 450

layout(location = 0) in Interface {
    vec3 eyeDirection;
    vec2 texCoord;
} fs_in;

layout(location = 0) out vec3 FragColor;

layout(binding = 0) uniform samplerCube skyBox;

void main() {
    FragColor = texture(skyBox, fs_in.eyeDirection).rgb;
}