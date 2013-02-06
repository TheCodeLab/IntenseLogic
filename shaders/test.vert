#version 140

in vec4 in_Position;
in vec2 in_Texcoord;
in vec3 in_Normal;
in vec3 in_Ambient;
in vec3 in_Diffuse;
in vec4 in_Specular;

out vec2 texcoord;
out vec3 normal;
out vec3 ambient;
out vec3 diffuse;
out vec4 specular;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * in_Position;
    texcoord = in_Texcoord;
    normal = in_Normal;
    ambient = in_Ambient;
    diffuse = in_Diffuse;
    specular = in_Specular;
}

