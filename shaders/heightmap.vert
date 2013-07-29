#version 140

in vec3 in_Position;
in vec3 in_Texcoord;

uniform sampler2D height_tex;
uniform mat4 mvp;
uniform mat4 imt;
uniform vec2 size;

out vec3 normal;

// http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
float h(vec2 t)
{
    return texture(height_tex, t/size).x;
}
void compute_normal()
{
    vec2 t = in_Texcoord.xy;
    vec2 d;
    float me;
    
    me = h(t);
    d.x = h(t - vec2(1, 0)) - me;
    d.y = h(t - vec2(0, 1)) - me;
    vec3 N = normalize(cross(vec3(0, d.y, -1/size.y), vec3(-1/size.x, d.x, 0)));
    vec4 normal4 = vec4(N, 0) * imt;
    normal = normalize(normal4.xyz);
}

void main()
{
    float height = texture(height_tex, in_Position.xy).x;
    gl_Position = mvp * vec4(in_Position.x, height, in_Position.y, 1.0);
    compute_normal();
}

