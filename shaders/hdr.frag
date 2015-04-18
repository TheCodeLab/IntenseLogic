#version 140

in vec2 texcoord;

uniform sampler2DRect tex;
uniform vec2 size;
uniform float exposure;
uniform float gamma;

out vec3 color;

void main() {
    vec3 col = texture(tex, texcoord * size).xyz;
    vec3 lum = vec3(0.30, 0.59, 0.11);
    float Y = dot(lum, col);
    const float exposure = 0.3;
    const float gamma = 0.5;
    col *= vec3(exposure * pow(Y, gamma));
    color = col;
}
