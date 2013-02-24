#version 140

in vec2 texcoord;
uniform sampler2DRect tex;
out vec3 color;

void main()
{
    color = texture(tex, texcoord * vec2(800, 600/*textureSize(tex)*/)).xyz;
}

