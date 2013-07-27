#version 140

in vec3 in_Position;
in vec3 in_Texcoord;

uniform sampler2D height_tex;
uniform mat4 mvp;
uniform vec2 size;

out vec3 normal;

// http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
float h(float x, float y)
{
    return texture(height_tex, vec2(x,y) / size).x;
}
void compute_normal()
{
    float x = in_Texcoord.x;
    float y = in_Texcoord.y;
    float me = h(x, y);
    float sx = h(x>0? x-1:x, y) - me;
    float sy = h(x, y>0? y-1:y) - me;
    normal = -normalize(cross(vec3(-1, sx, 0), vec3(0, sy, -1)));
}

void main()
{
    float height = texture(height_tex, in_Position.xy).x;
    gl_Position = mvp * vec4(in_Position.x, height, in_Position.y, 1.0);
    compute_normal();
}

