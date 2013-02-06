#version 140

in vec2 texcoord;
uniform sampler2DRect tex;
out vec3 color;

void main()
{
    color = vec3(texture(tex, texcoord * vec2(textureSize(tex))).xyz);
}

