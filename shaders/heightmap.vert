#version 140

in vec3 in_Position;

uniform sampler2D height_tex;
uniform mat4 mvp;
uniform vec2 size;

out vec3 normal;

// http://www.flipcode.com/archives/Calculating_Vertex_Normals_for_Height_Maps.shtml
/*void compute_normal()
{
    vec2 hstep = vec2(1) / size;
    vec2 p = in_Position.xy;
    float sx = h(p.x<1 ? p.x+hstep.x : p.x, p.y) - h(x0 ? x-1 : x, y);
    if (x == 0 || x == width-1) {
        sx *= 2;
    }
    float sy = h(x, y<height-1 ? y+1 : y) - h(x, y0 ?  y-1 : y);
    if (y == 0 || y == height -1) {
        sy *= 2;
    }
    normal = noramlize(vec3(-sx, 2, sy));
}*/

void main()
{
    float height = texture(height_tex, in_Position.xy).x;
    gl_Position = mvp * vec4(in_Position.x, height, in_Position.y, 1.0);
    normal = vec3(0, 1, 0);
}

