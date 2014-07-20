#version 140

in vec2 in_Position;
out vec2 position;

void main()
{
    gl_Position = vec4(in_Position * 2 - vec2(1), 0.0, 1.0);
    position = in_Position;
}
