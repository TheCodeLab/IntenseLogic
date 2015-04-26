#version 140

in vec4 in_Position;
in vec2 in_Texcoord;
in vec3 in_Normal;
in vec3 in_Ambient;
in vec4 in_Specular;

out vec2 texcoord;
out vec3 normal;

uniform mat4 mvp;
uniform mat4 imt;

void main()
{
    gl_Position = mvp * in_Position;
    texcoord = in_Texcoord;
    vec4 normal4 = vec4(in_Normal, 0.0) * imt;
    normal = normal4.xyz;
}
