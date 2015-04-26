#version 140

in vec3 normal;
in vec2 texcoord;

out vec3 out_Normal;
out vec4 out_Ambient;
out vec3 out_Diffuse;
out vec4 out_Specular;

uniform sampler2D ambient_tex;

void main()
{
    out_Normal = normal / vec3(2) + vec3(.5);
    vec3 col = texture(ambient_tex, texcoord).xyz;
    out_Diffuse = col;
    out_Specular = vec4(col, 96);
}
