#version 140

in ivec2 in_Position;
in ivec2 in_Texcoord;
out vec2 texcoord;

void main()
{
    texcoord = in_Texcoord;
    gl_Position = vec4(in_Position*2 - vec2(1.0), 0.0, 1.0);
}

