#version 450

layout (location = 0) out vec4 color;

layout (location = 0) in vec3 f_Position;
layout (location = 1) in vec3 f_Normal;
layout (location = 2) in vec2 f_UV;
layout (location = 3) in mat3 f_TBN;
layout (location = 6) in mat3 f_Normal_mat;

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

const float PI = 3.14159265359;

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

//vec3 calc_point_light(PointLight light, vec3 normal, vec3 view_direction, vec3 material_diffuse, vec3 material_specular);
vec3 calc_directional_light(vec3 view_direction, vec3 normal, float roughness, float metallic, vec3 F0, vec3 albedo);

float distribution_ggx(vec3 normal, vec3 halfway_vector, float roughness);
float geometry_schlick_ggx(float NdotV, float roughness);
float geometry_smith(vec3 normal, vec3 view_direction, vec3 light_direction, float roughness);
vec3 fresnel_schlick(float cosTheta, vec3 F0);

void main()
{
    vec4 albedo = u_Material.albedo;

    // Get normal either from normal map or from vertex
    vec3 normal = normalize(f_Normal_mat * f_Normal);

    // Get metallic float either from map or from uniform float
    float metallic = u_Material.metallic;

    // Get the ambient occlusion float either from the map or from uniform float
    float ao = u_Material.ambient_occlusion;

    // Get the roughness float either from the map or from uniform float
    float roughness = u_Material.roughness;

    vec3 view_direction = normalize(u_Camera.position.xyz - f_Position);

    // The Fresnel-Schlick approximation expects a F0 parameter which is known as the surface reflection at zero incidence
    // or how much the surface reflects if looking directly at the surface.
    //
    // The F0 varies per material and is tinted on metals as we find in large material databases.
    // In the PBR metallic workflow we make the simplifying assumption that most dielectric surfaces look visually correct with a constant F0 of 0.04.
    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo.rgb, metallic);

    // Output luminance accumulation
    vec3 output_luminance = calc_directional_light(view_direction, normal, roughness, metallic, F0, albedo.rgb);
    //    for (int i = 0; i <        8.2.  SDP Parameters.......................................... 52ub_Point_lights.count; i++)
    //    {
    //        // Create a point light that we can use from the uniform buffer
    //        PointLight light = PointLight(ub_Point_lights.positions[i].xyz,
    //        ub_Point_lights.constants[i].x,
    //        ub_Point_lights.linears[i].x,
    //        ub_Point_lights.quadratics[i].x,
    //        ub_Point_lights.ambients[i].xyz,
    //        ub_Point_lights.diffuses[i].xyz,
    //        ub_Point_lights.speculars[i].x,
    //        ub_Point_lights.intensities[i].x);
    //
    //        // Add to outgoing radiance
    //        output_luminance += calc_point_light(light, view_direction, normal, roughness, metallic, F0, albedo.rgb);//(kD * albedo.rgb / PI + specular) * radiance * cos_theta;
    //    }

    vec3 ambient = vec3(0.03) * albedo.rgb;
    vec3 result  = ambient + output_luminance;

    //    color     = vec4(result, albedo.a);
    color = vec4(1);
}

vec3 calc_directional_light(vec3 view_direction, vec3 normal, float roughness, float metallic, vec3 F0, vec3 albedo)
{
    // The light direction from the fragment position
    vec3 light_direction = normalize(-u_Directional_light.direction.xyz);
    vec3 halfway_vector  = normalize(view_direction + light_direction);

    // Add the radiance
    vec3 radiance         = u_Directional_light.diffuse.rgb * u_Directional_light.intensity;

    // Cook torrance BRDF
    float D         = distribution_ggx(normal, halfway_vector, roughness);
    float G         = geometry_smith(normal, view_direction, light_direction, roughness);
    vec3  F         = fresnel_schlick(clamp(dot(halfway_vector, view_direction), 0.0, 1.0), F0);

    vec3 kS         = F;
    vec3 kD         = vec3(1.0) - kS;
    kD                *= 1.0 - metallic;

    vec3 numerator       = D * G * F;
    float denominator    = 4.0 * max(dot(normal, view_direction), 0.0) * max(dot(normal, light_direction), 0.0);
    vec3 specular        = numerator / max(denominator, 0.001);

    // Get the cosine theta of the light against the normal
    float cos_theta      = max(dot(normal, light_direction), 0.0);

    return (kD * albedo.rgb / PI + specular) * radiance * cos_theta;
}

//vec3 calc_point_light(PointLight light, vec3 view_direction, vec3 normal, float roughness, float metallic, vec3 F0, vec3 albedo)
//{
//    // The light direction from the fragment position
//    vec3 light_direction = normalize(light.position.xyz - f_Position);
//    vec3 halfway_vector  = normalize(view_direction + light_direction);
//
//    // Point light attenuation
//    float distance       = length(light.position.xyz - f_Position);
//    float attenuation    = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance);
//
//    // Add the radiance
//    vec3 radiance         = light.diffuse.rgb * light.intensity * attenuation;// calc_point_light(point_light);
//
//    // Cook torrance BRDF
//    float D         = distribution_ggx(normal, halfway_vector, roughness);
//    float G         = geometry_smith(normal, view_direction, light_direction, roughness);
//    vec3  F         = fresnel_schlick(clamp(dot(halfway_vector, view_direction), 0.0, 1.0), F0);
//
//    vec3 kS         = F;
//    vec3 kD         = vec3(1.0) - kS;
//    kD                *= 1.0 - metallic;
//
//    vec3 numerator       = D * G * F;
//    float denominator    = 4.0 * max(dot(normal, view_direction), 0.0) * max(dot(normal, light_direction), 0.0);
//    vec3 specular        = numerator / max(denominator, 0.001);
//
//    // Get the cosine theta of the light against the normal
//    float cos_theta      = max(dot(normal, light_direction), 0.0);
//
//    return (kD * albedo.rgb / PI + specular) * radiance * cos_theta;
//}

float distribution_ggx(vec3 normal, vec3 halfway_vector, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(normal, halfway_vector), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom     = PI * denom * denom;

    return nom / max(denom, 0.0000001);
}

float geometry_schlick_ggx(float NdotV, float roughness)
{
    float r    = (roughness + 1.0);
    float k    = (r * r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float geometry_smith(vec3 normal, vec3 view_direction, vec3 light_direction, float roughness)
{
    float NdotV = max(dot(normal, view_direction), 0.0);
    float NdotL = max(dot(normal, light_direction), 0.0);
    float ggx2  = geometry_schlick_ggx(NdotV, roughness);
    float ggx1  = geometry_schlick_ggx(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnel_schlick(float cos_theta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cos_theta, 0.0), 5.0);
}