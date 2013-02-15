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
uniform mat4 vp;

void main()
{
    gl_Position = mvp * in_Position;
    texcoord = in_Texcoord;
    vec4 normal4 = vp * vec4(in_Normal, 1.0);
    normal = normal4.xyz / normal4.w;
    ambient = in_Diffuse;//in_Ambient;
    diffuse = in_Diffuse;
    specular = in_Specular;
}

