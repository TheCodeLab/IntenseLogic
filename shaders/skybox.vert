#version 140

in vec3 in_Position;

uniform mat4 mat;

out vec3 texcoord;

void main()
{
    gl_Position = mat * vec4(in_Position * 5, 1.0);
    texcoord = in_Position;
}

