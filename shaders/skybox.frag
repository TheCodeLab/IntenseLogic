#version 140

in vec3 texcoord;

uniform samplerCube skytex;

out vec3 out_Color;
out vec3 out_Normal;

void main()
{
    out_Normal = vec3(0.5);
    out_Color = texture(skytex, texcoord).xyz;
}
