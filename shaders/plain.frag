#version 140

out vec4 color;

void main()
{
    float f=1000.0;
    float n = 2.f;
    float z = (2 * n) / (f + n - gl_FragCoord.z * (f - n));
    color = vec4(z);
}

