#version 140

in vec2 texcoord;

uniform sampler2DRect tex;
uniform vec2 size;
const float bm = 0.5;
const float exposure = 1.0;

out vec3 color;

void main() {
    vec3 col = texture(tex, texcoord * size).xyz;
    float Y = dot(vec3(0.30, 0.59, 0.11), col);
    float YD = exposure * (exposure/bm + 1.0) / (exposure + 1.0);
    col *= YD;
    color = col;
}

