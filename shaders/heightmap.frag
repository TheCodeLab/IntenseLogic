#version 140

in vec3 normal;
in vec2 texcoord;

out vec3 out_Normal;
out vec3 out_Albedo;

uniform sampler2D ambient_tex;

void main()
{
    out_Normal = normal / vec3(2) + vec3(.5);
    vec3 col = texture(ambient_tex, texcoord).xyz;
    out_Albedo = col;
}
