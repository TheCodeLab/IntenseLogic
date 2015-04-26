#version 140

in vec2 texcoord;
in vec3 normal;
in vec3 ambient;
in vec4 specular;

out vec3 out_Diffuse;
out vec3 out_Normal;
out vec4 out_Specular;

uniform sampler2D tex;

void main()
{
    out_Normal = normal;
    out_Diffuse = texture(tex, texcoord).xyz;
    out_Specular = vec4(specular.xyz, specular.w);
}
