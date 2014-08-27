#version 140

in vec2 texcoord;

uniform sampler2DRect tex;
uniform vec2 size;

out vec3 out_Color;

const float bf = 1.0;

void main()
{
    const float[5] kernel = float[5](
        0.053991, 0.241971, 0.398942, 0.241971, 0.053991
    );
    vec3 sum = vec3(0);
    for (int i = 0; i < 5; i++) {
        sum += kernel[i] * texture(tex, texcoord*size + vec2(0, i-2)).xyz;
    }
    out_Color = sum * bf;
}
