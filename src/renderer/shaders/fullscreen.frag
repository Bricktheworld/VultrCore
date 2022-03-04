#version 450

layout (location = 0) out vec4 color;

layout (location = 0) in vec2 f_UV;

layout(set = 0, binding = 0) uniform sampler2D attachment;

void main()
{
    color = vec4(texture(attachment, f_UV).rgb, 1.0f);
}
