#version 140

in vec2 texcoord;
out vec4 color;
uniform sampler2D tex;

void main()
{
    color = vec4(1.0); //texture(tex, texcoord);
}

