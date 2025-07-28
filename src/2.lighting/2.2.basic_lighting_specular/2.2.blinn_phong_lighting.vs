#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 ViewPos;  // To calculate view direction in fragment shader

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 viewPosition;  // Camera position

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  // Normal in world space
    ViewPos = viewPosition;  // Pass camera position to fragment shader
}