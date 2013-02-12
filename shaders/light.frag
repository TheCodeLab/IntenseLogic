#version 140

//uniform LightBlock {
    uniform vec3 position;//[1024];
    uniform vec3 color;//[1024];
    uniform float radius;//[1024];
//};
uniform sampler2DRect depth;
//uniform sampler2DRect accumulation;
uniform sampler2DRect normal;
uniform sampler2DRect diffuse;
uniform sampler2DRect specular;
uniform vec3 camera;
uniform mat4 mvp;
uniform mat4 ivp;

flat in int instanceID;

out vec3 out_Color;

void main() {
    vec4 pos_xyzw = ivp * vec4(gl_FragCoord.xy, texture(depth, gl_FragCoord.xy).x, 1.0);
    vec3 pos = pos_xyzw.xyz / pos_xyzw.w;
    vec3 light_dir = normalize(position/*[instanceID]*/ - pos);
    vec3 norm = texture(normal, gl_FragCoord.xy).xyz;
    out_Color = vec3(0);
    out_Color += texture(diffuse, gl_FragCoord.xy).xyz * max(0, dot(light_dir, norm));
    vec3 user_dir = normalize(camera - pos);
    vec3 ray = normalize(2 * dot(light_dir, norm) * norm - light_dir);
    vec4 specular = texture(specular, gl_FragCoord.xy);
    out_Color += specular.xyz * pow(max(0, dot(ray, user_dir)), specular.w);
    //out_Color += vec3(0, 0, .3); // blue hint
    //out_Color = vec3(1.0);
}

