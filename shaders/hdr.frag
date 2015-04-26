#version 140

in vec2 texcoord;

uniform sampler2DRect tex;
uniform vec2 size;
uniform float exposure;
uniform float gamma;

out vec3 out_Color;

void main() {
    vec3 col = texture(tex, texcoord * size).xyz;
    vec3 lum = vec3(0.30, 0.59, 0.11);
    float Y = dot(lum, col);
    float U = 0.492 * (col.b - Y);
    float V = 0.877 * (col.r - Y);
    float Y2 = exposure * pow(Y, gamma);
    out_Color = vec3(Y2 + 1.14*V, Y2 - 0.395*U - 0.581*V, Y2 + 2.033*U);
    //out_Color = vec3(exposure) * pow(col, vec3(gamma));
}
