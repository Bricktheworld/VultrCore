#version 450

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;
layout (location = 2) in vec2 v_UV;
layout (location = 3) in vec3 v_Tangent;
layout (location = 4) in vec3 v_Bitangent;

layout (location = 0) out vec2 f_UV;

layout(set = 0, binding = 0) uniform sampler2D attachment;

void main()
{
    gl_Position = vec4(v_Position, 1.0);
    f_UV        = v_UV;
}
