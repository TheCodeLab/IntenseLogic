#version 140

in vec2 share_texcoord;
in vec3 share_normal;

out vec3 out_Albedo;
out vec3 out_Normal;
out float out_Refraction;
out float out_Gloss;
out float out_Emission;

uniform sampler2D tex_Albedo;
uniform sampler2D tex_Normal;
uniform sampler2D tex_Refraction;
uniform sampler2D tex_Emission;

void main()
{
    vec3 albedo = texture(tex_Albedo, share_texcoord).xyz;
    vec3 emission = texture(tex_Emission, share_texcoord).xyz;
    float emission_l = dot(vec3(0.30, 0.59, 0.11), emission);
    out_Albedo = mix(albedo, emission, emission_l);
    out_Normal = share_normal;
    out_Refraction = texture(tex_Refraction, share_texcoord).x * 2.0;
    out_Gloss = 50.0;
    out_Emission = emission_l;
}
