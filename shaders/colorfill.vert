#version 140

in vec2 in_Position;
uniform vec4 position;

void main()
{
    gl_Position = vec4(in_Position * position.zw * vec2(2) + position.xy - vec2(1), 0.f, 1.f);
}

