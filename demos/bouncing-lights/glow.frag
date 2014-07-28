#version 140

out vec3 out_Normal;
out vec3 out_Ambient;
out vec3 out_Diffuse;
out vec4 out_Specular;
uniform vec3 col;

void main()
{
    out_Normal = vec3(0);
    out_Ambient = col;
    out_Diffuse = vec3(1);
    out_Specular = vec4(1,1,1, 96/255.0);
}
