// See http://www.ozone3d.net/tutorials/glsl_texturing_p04.php for ref

#ifndef GL_ARB_bindless_texture
uniform sampler2D tex;
#endif

#ifdef GL_ARB_bindless_texture
flat in sampler2D handle;
#endif
in vec3 nor;
out vec4 FragColor;


vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);
vec3 getLightFactor(float specMapValue);

void main() {
    vec3 texc = gl_FragCoord.xyz / vec3(screen, 1.);
    vec3 u = getPosFromUVDepth(texc, InverseProjectionMatrix).xyz;
    vec3 r = reflect(u, nor);

    float m = 2.0 * sqrt(r.x * r.x + r.y * r.y + (r.z + 1.0) * (r.z + 1.0));
    r.y = - r.y;
#ifdef GL_ARB_bindless_texture
    vec4 detail0 = texture(handle, r.xy / m + .5);
#ifdef SRGBBindlessFix
    detail0.xyz = pow(detail0.xyz, vec3(2.2));
#endif
#else
    vec4 detail0 = texture(tex, r.xy / m + .5);
#endif
    vec3 LightFactor = getLightFactor(1.);

    FragColor = vec4(detail0.xyz * LightFactor, 1.);
}
