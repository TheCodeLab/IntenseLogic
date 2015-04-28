#version 140

uniform vec3 color;
uniform float radius;
uniform sampler2DRect depth;
uniform sampler2DRect normal;
uniform sampler2DRect albedo;
uniform sampler2DRect reflected;
uniform sampler2DRect gloss;
uniform mat4 mv;
uniform mat4 ivp;
uniform vec2 size;

out vec3 out_Color;

/*
[17:03] <feep> so device = clip.xyz/w and clip = Matrix world
[17:03] <feep> and world.w = 1, solve for world, device is given.
[17:04] <feep> (Matrix is assumed constant)
[17:04] <feep> clip = Matrix world, so Matrix^-1 clip = Matrix^-1 Matrix world = world
[17:04] <feep> device = clip.xyz / clip.w, clip.xyz = device * clip.w
[17:05] <feep> Matrix^-1 (clip.xyz clip.w) = world
[17:05] <feep> Matrix^-1 (device * clip.w   clip.w) = world
[17:05] <feep> Matrix^-1 (clip.w (device   1)) = world
[17:05] <feep> matrices are linear transforms, M f a = f M a
[17:06] <feep> clip.w Matrix^-1 (device  1) = world
[17:06] <feep> are we stuck? no! world.w = 1.
[17:06] <feep> so take that last line and isolate the w component
[17:06] <feep> clip.w (Matrix^-1 (device 1)).w = world.w = 1
[17:07] <feep> clip.w = 1 / (Matrix^-1 (device 1)).w
[17:08] <feep> (Matrix^-1 (device  1)) / (Matrix^-1 (device 1)).w = world
[17:08] <feep> iow
[17:08] <feep> world = (Matrix^-1 (device  1)).(xyz/w)
*/
vec3 screen_to_world(vec3 sp)
{
    vec4 res = ivp * vec4(sp, 1);
    return res.xyz/res.w;
}

vec3 get_lightpos()
{
    vec4 res = mv * vec4(0,0,0, 1);
    return res.xyz / res.w;
}

vec4 my_sample(sampler2DRect s, ivec2 uv)
{
    return texture(s, uv);
}

void main()
{
    ivec2 fc = ivec2(gl_FragCoord.xy - vec2(.5));
    float depth = my_sample(depth, fc).x;
    vec3 norm = normalize(my_sample(normal, fc).xyz);
    vec3 albedo = my_sample(albedo, fc).xyz;
    float reflectivity = clamp(my_sample(reflected, fc).x, 0, 1);
    float gloss = my_sample(gloss, fc).x;

    // gl_FragCoord is from (.5, .5) to (w - .5, h - .5), depth texture is 0..1, feep's function wants (0,0,-1)..(1,1,1)
    vec3 pos = screen_to_world(vec3(gl_FragCoord.xy / size, depth) * 2 - 1);
    vec3 lightpos = get_lightpos();
    vec3 lightdir = normalize(lightpos - pos);
    float dist = length(lightpos - pos) / radius;
    float daf = max(0, 1 - dist);

    vec3 col = vec3(0);
    col += (1-reflectivity) * albedo * vec3(max(0, dot(lightdir, norm)));
    vec3 reflection = normalize(reflect(norm, lightdir));
    vec3 viewer = normalize(-pos); // camera space
    col += reflectivity * albedo * clamp(pow(max(0, dot(reflection, viewer)), gloss), 0, 1);

    out_Color = max(vec3(0), col * daf * color);
}
