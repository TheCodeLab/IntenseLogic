#version 140

in vec4 in_Position;
in vec2 in_Texcoord;
uniform mat4 mvp;
out vec2 texcoord;

void main()
{
    gl_Position = mvp * in_Position;
    texcoord = in_Texcoord;
}

