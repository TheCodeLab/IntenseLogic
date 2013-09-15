#version 140

in vec3 texcoord;

uniform samplerCube skytex;

out vec3 out_Color;
out vec3 out_Normal;
out vec3 out_Diffuse;
out vec4 out_Specular;

void main()
{
    out_Color = texture(skytex, texcoord).xyz;
    out_Normal = vec3(0);
    out_Diffuse = vec3(0);
    out_Specular = vec4(0);
}

