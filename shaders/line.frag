#version 140

out vec3 out_Ambient;

uniform vec3 col;

void main()
{
    out_Ambient = col;
}
