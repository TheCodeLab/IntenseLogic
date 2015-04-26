#version 140

out vec3 out_Normal;
out vec3 out_Albedo;
uniform vec3 col;

void main()
{
    out_Normal = vec3(0.5);
    out_Albedo = col;
}
