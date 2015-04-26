#version 140

in vec2 texcoord;
in vec3 normal;

out vec3 out_Albedo;
out vec3 out_Normal;
out float out_Reflect;
out float out_Gloss;

uniform sampler2D tex;

void main()
{
    out_Normal = normal;
    out_Albedo = texture(tex, texcoord).xyz;
    out_Reflect = 0.5;
    out_Gloss = 1.0;
}
