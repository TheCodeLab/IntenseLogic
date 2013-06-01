#version 140

in vec3 in_Position;

uniform mat4 mvp;
uniform float radius;

void main()
{
    vec4 pos = mvp * vec4(in_Position * radius, 1.0);
    gl_Position = pos;
}

