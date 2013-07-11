#version 140

in vec2 texcoord;
in vec3 normal;
in vec3 ambient;
in vec3 diffuse;
in vec4 specular;

out vec3 out_Normal;
out vec4 out_Ambient;
out vec3 out_Diffuse;
out vec4 out_Specular;

uniform sampler2D tex;

void main()
{
    out_Normal = normal;
    out_Ambient = /*vec4(normal/2 + vec3(.5), 1.0);*/texture(tex, texcoord); //* vec4(ambient, 1.0); 
    out_Diffuse = diffuse;
    out_Specular = specular;
    //color = texture(tex, texcoord) * (sin(texcoord.x) + cos(texcoord.y))/2;
}

