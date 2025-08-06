#version 330 core

layout (location = 0) in vec3 aPos;     // Position
layout (location = 1) in vec3 aNormal;  // Normal
layout (location = 2) in vec3 aColor;   // Color


out vec3 FragPos;    // To fragment shader: world-space position
out vec3 Normal;     // To fragment shader: world-space normal
out vec3 VertexColor;// To fragment shader: color

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Compute world-space position of this vertex
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;

    // Transform normal by the inverse-transpose of model
    Normal = mat3(transpose(inverse(model))) * aNormal;

    //Keep color the same for each vertex
    VertexColor = aColor;

    // Standard projection
    gl_Position = projection * view * worldPos;
}