#version 140

out vec3 out_Color;
uniform vec3 color;
uniform sampler2DRect tex_Albedo;
uniform sampler2DRect tex_Emission;

void main()
{
    vec3 albedo = texture(tex_Albedo, gl_FragCoord.xy).xyz;
    float emission = texture(tex_Emission, gl_FragCoord.xy).x;
    // TODO: Explicitly encode the luminance of ambient lighting
    out_Color = color * albedo + albedo * emission;
}
