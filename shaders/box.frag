#version 140

in vec3 position;
out vec3 out_Color;

void main()
{
    out_Color = position + vec3(1) / vec3(2);
}
