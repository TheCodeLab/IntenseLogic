#version 140

in vec3 normal;
in vec2 texcoord;

out vec3 out_Normal;
out vec4 out_Ambient;
out vec3 out_Diffuse;
out vec4 out_Specular;

uniform sampler2D ambient_tex;

void main()
{
    out_Normal = normal;
    out_Ambient = texture(ambient_tex, texcoord); //vec4(0, 0, 0, 1); //vec4(.5, .5, .5, 1);
    out_Diffuse = vec3(.64);
    out_Specular = vec4(.5, .5, .5, 96);
}

