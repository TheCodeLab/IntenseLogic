#version 140

in vec2 texcoord;

uniform sampler2DRect tex;
uniform vec2 size;
//uniform float co;
const float co = 2.0;

out vec3 color;

vec3 blur() {
    const float[5*5] kernel = float[5*5](
0.000722, 0.006262, 0.012865, 0.006262, 0.000722, 
0.006262, 0.0543, 0.111555, 0.0543, 0.006262, 
0.012865, 0.111555, 0.229183, 0.111555, 0.012865, 
0.006262, 0.0543, 0.111555, 0.0543, 0.006262, 
0.000722, 0.006262, 0.012865, 0.006262, 0.000722);
    vec3 sum = vec3(0);
    for (float j = 0; j < 4; j++) {
        for (int i = 0; i < 5*5; i++) {
            sum += kernel[i] * max(vec3(0), texture(tex, texcoord*size + vec2((i%5) - 2, i/5 - 2)*(1+j)).xyz - 1/co);
        }
    }
    return sum;
}

void main() {
    vec3 col = texture(tex, texcoord * size).xyz;
    color = co * col + blur();
}

