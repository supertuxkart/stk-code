vec3 applyFog(
    vec3  eyedir,
    vec3  sundir,
    vec3  color,
    vec3  sun_color, 
    float sun_scatter, 
    float dist,
    vec4  fog_color, 
    float fog_density)
{
    float scattering = pow(max(dot(eyedir, sundir), 0.), 8.) * sun_scatter;
    vec4 fog_color_adjusted = fog_color + vec4(scattering * sun_color, 0.);
    fog_color_adjusted.a *= 1.0 - exp(-dist * fog_density);
    return mix(color, fog_color_adjusted.rgb, fog_color_adjusted.a);
}