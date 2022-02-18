#version 450

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;
layout (location = 2) in vec2 v_UV;
layout (location = 3) in vec3 v_Tangent;
layout (location = 4) in vec3 v_Bitangent;

layout (location = 0) out vec3 f_Position;
layout (location = 1) out vec3 f_Normal;
layout (location = 2) out vec2 f_UV;
layout (location = 3) out mat3 f_TBN;
layout (location = 6) out mat3 f_Normal_matrix;

layout(push_constant) uniform Constants
{
    vec4 color;
    mat4 mat;
} u_Model;

layout (set = 0, binding = 0) uniform Camera {
    vec4 position;
    mat4 view;
    mat4 proj;
    mat4 view_proj;
} u_Camera;

layout (set = 0, binding = 1) uniform DirectionalLight {
    vec4 direction;

    vec4 ambient;
    vec4 diffuse;
    float specular;
    float intensity;
    int exists;
} u_Directional_light;

layout (set = 1, binding = 0) uniform Material {
    vec4 albedo;
    float metallic;
    float ambient_occlusion;
    float roughness;
} u_Material;

void main()
{
    mat4 mvp    = u_Camera.view_proj * u_Model.mat;
    mat3 normal_mat = mat3(transpose(inverse(u_Model.mat)));
    gl_Position = mvp * vec4(v_Position, 1.0f);
    f_Position  = vec3(u_Model.mat * vec4(v_Position, 1.0f));
    f_Normal    = v_Normal;
    f_UV        = v_UV;
    f_Normal_matrix = normal_mat;


    // TBN matrix calculation
    vec3 T = normalize(normal_mat * v_Tangent);
    vec3 N = normalize(normal_mat * v_Normal);

    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    f_TBN = mat3(T, B, N);
}