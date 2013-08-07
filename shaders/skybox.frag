#version 140

in vec3 texcoord;

uniform samplerCube skytex;

out vec3 out_Color;

void main()
{
    out_Color = texture(skytex, texcoord).xyz;
}

