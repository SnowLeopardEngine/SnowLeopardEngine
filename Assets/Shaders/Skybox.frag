#version 460 core

in vec2 varingTexCoords;
in vec3 fragPos;
in vec3 Normal;

out vec4 FragColor;

void main() {
    vec3 color = vec3(1, 1, 1);
    
    if(abs(fragPos.x) > abs(fragPos.y) && abs(fragPos.x) > abs(fragPos.z)) 
    {
        color = fragPos.x > 0 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    } 

    else if(abs(fragPos.y) > abs(fragPos.x) && abs(fragPos.y) > abs(fragPos.z)) 
    {
        color = fragPos.y > 0 ? vec3(0.9, 0.9, 0.9) : vec3(0.9, 0.9, 0.9);
    } 

    else if(abs(fragPos.z) > abs(fragPos.x) && abs(fragPos.z) > abs(fragPos.y)) 
    {
        color = fragPos.z > 0 ? vec3(0.7, 0.7, 0.7) : vec3(0.7, 0.7, 0.7);
    }

    FragColor = vec4(color, 1);
}