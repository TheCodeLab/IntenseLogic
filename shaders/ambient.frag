#version 140

out vec3 out_Color;
uniform vec3 color;
uniform sampler2DRect tex_Albedo;
uniform sampler2DRect tex_Emission;
uniform float fovsquared;

void main()
{
    vec3 albedo = texture(tex_Albedo, gl_FragCoord.xy).xyz;
    float emission = texture(tex_Emission, gl_FragCoord.xy).x;
    // Emission is explicitly encoded as radiant intensity, and color
    // has it implicitly encoded. The output must be in irradiance. We
    // assume that the sensor is the unit size (1 m^2), which means we
    // only need to account for its view cone, which is the field of
    // view in radians squared.
    out_Color = color * albedo * fovsquared + albedo * emission * fovsquared;
}
