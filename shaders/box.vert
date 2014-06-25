#version 140

in vec3 in_Position;
out vec3 position;
uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(in_Position, 1.0);
    position = in_Position;
}
