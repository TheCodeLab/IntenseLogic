#version 140

in vec3 in_Position;

uniform MVPBlock {
    mat4 mvp[1024];
};

void main()
{
    gl_Position = mvp[gl_InstanceID] * vec4(in_Position, 1.0);
}

