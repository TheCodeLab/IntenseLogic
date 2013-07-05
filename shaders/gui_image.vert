#version 140

in vec2 in_Position;

uniform vec2 pos1;
uniform vec2 pos2;

out vec2 texcoord;

void main()
{
    gl_Position = vec4(2 * ((pos2 - pos1) * in_Position + pos1) - vec2(1), 0.0, 1.0);
    texcoord = in_Position * vec2(1, -1);
}

