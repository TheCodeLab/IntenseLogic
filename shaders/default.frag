#version 140

in vec2 in_Texcoord;
out vec4 color;
uniform sampler2D tex;

void main()
{
    color = texture(tex, in_Texcoord);
}

