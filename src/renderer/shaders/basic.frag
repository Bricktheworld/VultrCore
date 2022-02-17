#version 450

layout (location = 0) out vec4 color;

layout (location = 0) in vec3 f_Position;
layout (location = 1) in vec3 f_Normal;
layout (location = 2) in vec2 f_UV;
layout (location = 3) in mat3 f_TBN;

layout (set = 0, binding = 0) uniform Camera {
    vec4 position;
    mat4 view;
    mat4 proj;
    mat4 view_proj;
} u_Camera;

const float PI = 3.14159265359;

void main()
{
    color = vec4(1);
}