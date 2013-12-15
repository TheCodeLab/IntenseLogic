#version 140

in vec3 in_Position;
in vec3 in_Ambient;

uniform mat4 vp;

out vec3 color;

void main()
{
    gl_Position = vp * vec4(in_Position, 1.0);
    color = in_Ambient;
}

