#version 140

in vec2 position;
out vec3 out_Color;

void main()
{
    vec3 col = vec3(0);
    col += mix(vec3(1, 0, 0), vec3(0, 1, 0), position.x);
    col += mix(vec3(0, 1, 0), vec3(1, 0, 1), position.y);
    out_Color = col;
}
