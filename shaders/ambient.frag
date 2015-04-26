#version 140

out vec3 out_Color;
uniform vec3 color;
uniform sampler2DRect tex;

void main()
{
    out_Color = color * texture(tex, gl_FragCoord.xy).xyz;
}
