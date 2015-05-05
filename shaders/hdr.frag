#version 140

in vec2 texcoord;

uniform sampler2DRect tex;
uniform vec2 size;
uniform float exposure;
uniform float gamma;

out vec3 out_Color;

void main() {
    vec3 col = texture(tex, texcoord * size).xyz;
    mat3 rgb_to_xyz = mat3(
        0.4124, 0.3576, 0.1805,
        0.2126, 0.7152, 0.0722,
        0.0193, 0.1192, 0.9505);
    mat3 xyz_to_rgb = mat3(
        3.2406, -1.5372, -0.4986,
        -0.9689, 1.8758, 0.0415,
        0.0557, -0.2040, 1.0570);
    col = rgb_to_xyz * col;
    float L = (col.x + col.y + col.z);
    float x = col.x / L;
    float y = col.y / L;
    float L2 = exposure * pow(L, gamma);

    float X = x * (L2 / y);
    float Z = (1 - x - y) * (L2 / y);
    col = vec3(X, L2, Z);
    out_Color = xyz_to_rgb * col;
}
