#version 140

in vec3 in_Position;
in vec3 in_Normal;
in vec2 in_Texcoord;

out vec2 share_texcoord;
out vec3 share_normal;

uniform mat4 mvp;
uniform mat4 imt;

void main()
{
    gl_Position = mvp * vec4(in_Position * vec3(1,4,1), 1.0);
    share_texcoord = in_Texcoord;
    vec4 normal4 = imt * vec4(in_Normal, 0.0);
    share_normal = normal4.xyz / normal4.w;
}
