#version 140

//uniform LightBlock {
    uniform vec3 position;//[1024]; // world space
    uniform vec3 color;//[1024];
    uniform float radius;//[1024];
//};
uniform sampler2DRect depth; // screen space
//uniform sampler2DRect accumulation;
uniform sampler2DRect normal; // world space
uniform sampler2DRect diffuse;
uniform sampler2DRect specular;
uniform vec3 camera; // world space
uniform mat4 mvp;
uniform mat4 ivp;

flat in int instanceID;

out vec3 out_Color;

vec3 calc_diffuse(vec3 pos, vec3 norm, vec3 dir, float daf)
{
    return /*texture(diffuse, gl_FragCoord.xy).xyz*/ vec3(1) * max(0, dot(dir, norm)) * daf;
}

vec3 calc_specular(vec3 pos, vec3 norm, vec3 dir, float daf)
{
    vec4 spec = texture(specular, gl_FragCoord.xy);
    vec3 ray = normalize(2 * dot(dir, norm) * norm - dir);
    vec3 user_dir = normalize(camera - pos);
    return spec.xyz * pow(max(0, dot(ray, user_dir)), spec.w) * daf;
}

vec3 window_to_camera()
{
    float Dn = 2;
    float Df = 1000; // TODO: uniform value for the near/far clipping
    vec2 V = vec2(800, 600); // TODO: window size uniform
    vec3 N;
    N.x = (2*gl_FragCoord.x)/V.x - 1;
    N.y = (2*gl_FragCoord.y)/V.y - 1;
    N.z = (2*gl_FragCoord.z - Df - Dn)/(Df-Dn);
    vec4 C = vec4(N, 1.0) / gl_FragCoord.w;
    vec4 res = ivp * C;
    return res.xyz/res.w;
}

void main() 
{
    //vec4 pos_xyzw = ivp * vec4(gl_FragCoord.xy/vec2(400,300)-vec2(400, 300), texture(depth, gl_FragCoord.xy).x, 1.0);
    vec3 pos = window_to_camera();//pos_xyzw.xyz / pos_xyzw.w; // world space
    vec3 light_dir = normalize(pos - position); // world space
    float daf = 1;//length(pos - position) / radius;
    vec3 norm = texture(normal, gl_FragCoord.xy).xyz;

    //out_Color = vec3(length(pos - position)/radius);
    out_Color = calc_diffuse(pos, norm, light_dir, daf); //+ calc_specular(pos, norm, light_dir, daf);

    if (length(pos - position) < 1) {
        out_Color = vec3(1, 0, 0);
    }
}

