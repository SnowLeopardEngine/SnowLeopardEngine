#version 450

out gl_PerVertex {
    vec4 gl_Position;
};
layout(location = 0) out vec2 varyTexCoords;

// glDrawArrays(GL_TRIANGLES, 3);
void main() {
    varyTexCoords = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(varyTexCoords * 2.0 - 1.0, 0.0, 1.0);
}