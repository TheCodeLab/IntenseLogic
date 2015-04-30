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
    float Y2 = exposure * pow(Y, gamma);
    float X = Y2 / Y;
    out_Color = col * (vec3(1) + vec3(X) * lum);
    //out_Color = vec3(exposure) * pow(col, vec3(gamma));
}
