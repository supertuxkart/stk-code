// Sun Most Representative Point (used for MRP area lighting method)
// From "Frostbite going PBR" paper

vec3 sunDirection(vec3 R, vec3 sun_direction, float sun_angle_tan_half, mat4 inverse_view_matrix)
{
    sun_direction = normalize((transpose(inverse_view_matrix) * vec4(sun_direction, 0.)).xyz);
    float DdotR = dot(sun_direction, R);
    vec3 S = normalize(R - DdotR * sun_direction);
    float sun_angle_tan_half2 = 1 + sun_angle_tan_half * sun_angle_tan_half;
    vec2 sun_angle_sin_cos = vec2(2 * sun_angle_tan_half, 2 - sun_angle_tan_half2) / sun_angle_tan_half2;
    // Equivalent to DdotR < cos(sun_angle)
    float factor = step(DdotR, sun_angle_sin_cos.y);
    return mix(R, normalize(sun_direction * sun_angle_sin_cos.y + S * sun_angle_sin_cos.x), factor);
}
