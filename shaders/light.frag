#version 140

uniform LightBlock {
    vec3 color[1024];
    float radius[1024];
};
uniform sampler2DRect depth;
//uniform sampler2DRect accumulation;
uniform sampler2DRect normal;
uniform sampler2DRect diffuse;
uniform sampler2DRect specular;

out vec3 out_Color;

void main() {
    // http://www.realtimerendering.com/blog/deferred-lighting-approaches/
    // l_o(v) = sum(k=1, n, f_shade(B_L_k, l_k, v, n, c_diff, c_spec, m)
    // each n is an invocation of the fragment shader at that pixel
    // l_o(v) = pixel
    // n = number of lights
    // B_L = RGB-HDR light color times its intensity times the distance attenuation factor at the shaded point
    // l = per-light direction
    // v = view vector
    // n = normal
    // c_diff = diffuse
    // c_spec = specular
    // m = specular coefficient

    out_Color = vec3(1.0);
}

