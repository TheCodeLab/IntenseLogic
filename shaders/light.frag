#version 140

uniform vec3 position;
uniform vec3 color;
uniform float radius;
uniform vec3 camera;
uniform sampler2DRect depth;
uniform sampler2DRect normal;
uniform sampler2DRect diffuse;
uniform sampler2DRect specular;
uniform mat4 mvp;
uniform mat4 ivp;

out vec3 out_Color;
out vec3 out_Normal;
out vec3 out_Diffuse;
out vec3 out_Specular;

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

void main() 
{
    // gl_FragCoord is from (.5, .5) to (w - .5, h - .5), depth texture is 0..1, feep's function wants (0,0,-1)..(1,1,1)
    vec3 pos = screen_to_world(vec3(gl_FragCoord.xy / vec2(800,600), texture(depth, gl_FragCoord.xy - vec2(.5)).x) * 2 - 1);
    vec3 light_dir = normalize(position - pos);
    vec3 norm = texture(normal, gl_FragCoord.xy).xyz;
    float daf = 1 - length(position - pos) / radius;

    vec3 col;
    vec3 diffuse = vec3(.64); //texture(diffuse, gl_FragCoord.xy - vec2(.5)).xyz; 
    col = diffuse * vec3(max(0, dot(light_dir, norm)));
    vec4 spec = vec4(.5, .5, .5, 96); //texture(specular, gl_FragCoord.xy - vec2(.5));
    col += spec.xyz * pow(max(0, dot(normalize(2 * dot(light_dir, norm) * norm - light_dir), normalize(camera - pos))), spec.w);

    out_Color = col * daf * color;
    out_Normal = vec3(0);
    out_Diffuse = vec3(0);
    out_Specular = vec3(0);
}

