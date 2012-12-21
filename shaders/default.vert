#version 140

in vec4 in_Position;
uniform mat4 mvp;

void main()
{
    gl_Position = mvp * in_Position;
}

