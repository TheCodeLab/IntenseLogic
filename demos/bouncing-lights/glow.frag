#version 140

out vec3 out_Normal;
out vec3 out_Diffuse;
out vec4 out_Specular;
uniform vec3 col;

void main()
{
    out_Normal = vec3(0.5);
    out_Diffuse = col;
    out_Specular = vec4(col, 96/255.0);
}
