// Sun Most Representative Point (used for MRP area lighting method)
// From "Frostbite going PBR" paper
vec3 SunMRP(vec3 normal, vec3 eyedir)
{
    vec3 local_sundir = normalize((transpose(InverseViewMatrix) * vec4(sun_direction, 0.)).xyz);
    vec3 R = reflect(-eyedir, normal);
    float angularRadius = 3.14 * sun_angle / 180.;
    vec3 D = local_sundir;
    float d = cos(angularRadius);
    float r = sin(angularRadius);
    float DdotR = dot(D, R);
    vec3 S = R - DdotR * D;
    return (DdotR < d) ? normalize(d * D + normalize (S) * r) : R;
}