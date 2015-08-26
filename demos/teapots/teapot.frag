#version 140

in vec2 texcoord;
in vec3 normal;

out vec3 out_Albedo;
out vec3 out_Normal;
out float out_Refraction;
out float out_Gloss;

uniform sampler2D tex;

void main()
{
    out_Normal = normal;
    out_Albedo = texture(tex, texcoord).xyz;
    out_Refraction = 2.7;
    out_Gloss = 96.0;
}
