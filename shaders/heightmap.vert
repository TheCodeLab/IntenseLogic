#version 140

in vec3 in_Position;
in vec3 in_Texcoord;

uniform sampler2D height_tex;
uniform sampler2D normal_tex;
uniform mat4 mvp;
uniform mat4 imt;
uniform vec2 size;

out vec3 normal;
out vec2 texcoord;

void compute_normal()
{
    vec4 normal4 = vec4(texture(normal_tex, in_Position.xy).xyz, 0) * imt;
    normal = normalize(normal4.xyz);
}

void main()
{
    float height = texture(height_tex, in_Position.xy).x - .5;
    gl_Position = mvp * vec4(in_Position.x - .5, height, in_Position.y - .5, 1.0);
    compute_normal();
    texcoord = in_Position.xy;
}
