#version 140

in vec2 texcoord;

uniform sampler2DRect tex;
uniform vec2 size;
uniform float exposure;

out vec3 out_Color;

void main()
{
    const float[5] kernel = float[5](
        0.053991, 0.241971, 0.398942, 0.241971, 0.053991
    );
    vec3 sum = vec3(0);
    float bm = 1.0 / exposure;
    for (int i = 0; i < 5; i++) {
        sum += kernel[i] * max(vec3(0), texture(tex, texcoord*size + vec2(i-2, 0)).xyz - bm);
    }
    out_Color = sum;
}
