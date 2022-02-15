#version 330 core
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;
layout (location = 2) in vec2 v_UV;
layout (location = 3) in vec3 v_Tangent;
layout (location = 4) in vec3 v_Bitangent;

out vec3 f_Normal;
out vec3 f_Position;
out vec2 f_UV;
out mat3 f_TBN;

layout(push_constant) uniform constants
{
    vec4 color;
    mat4 model_matrix;
} u_Push_constants;

//layout (std140) uniform Camera {
//    vec4 position;
//    mat4 view_matrix;
//    mat4 projection_matrix;
//} ub_Camera;
//
//uniform mat4 u_MVP;
//uniform mat4 u_Model_matrix;
//uniform mat3 u_Normal_matrix;

void main()
{
    gl_Position = u_MVP * vec4(v_Position, 1.0f);
    f_Position  = vec3(u_Model_matrix * vec4(v_Position, 1.0f));
    f_Normal    = v_Normal;
    f_UV    = v_UV;


    // TBN matrix calculation
    vec3 T = normalize(u_Normal_matrix * v_Tangent);
    vec3 N = normalize(u_Normal_matrix * v_Normal);
    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    f_TBN = mat3(T, B, N);
}