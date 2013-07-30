#version 140

in vec2 texcoord;
uniform sampler2DRect tex;
uniform vec2 size;
out vec3 color;

void main()
{
    color = texture(tex, texcoord * size).xyz;
}

