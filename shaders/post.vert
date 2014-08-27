#version 140

in vec2 in_Texcoord;
out vec2 texcoord;

void main()
{
    texcoord = in_Texcoord;
    gl_Position = vec4(in_Texcoord*2 - vec2(1.0), 0.0, 1.0);
}
