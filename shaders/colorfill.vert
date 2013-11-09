#version 140

in vec2 in_Position;
uniform vec4 position;

void main()
{
    vec2 pos1 = position.xy;
    vec2 pos2 = position.zw;
    gl_Position = vec4((2 * ((pos2 - pos1) * in_Position + pos1) - vec2(1)) * vec2(1, -1), 0.0, 1.0);
    //gl_Position = vec4(in_Position * position.zw * vec2(2) + position.xy - vec2(1), 0.f, 1.f);
}

