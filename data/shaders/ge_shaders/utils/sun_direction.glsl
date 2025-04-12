vec3 sunDirection(vec3 normal, vec3 eyedir, vec3 sundirection)
{
    vec3 local_sundir = normalize((transpose(u_camera.m_inverse_view_matrix) * vec4(sundirection, 0.)).xyz);
    vec3 R = reflect(-eyedir, normal);
    float angularRadius = 3.14 * 5. / 180.;
    vec3 D = local_sundir;
    float d = cos(angularRadius);
    float r = sin(angularRadius);
    float DdotR = dot(D, R);
    vec3 S = R - DdotR * D;
    return (DdotR < d) ? normalize(d * D + normalize (S) * r) : R;
}
