uniform samplerCube tex;
uniform mat4 invproj;
uniform vec2 screen;
uniform mat4 TransposeViewMatrix;

#if __VERSION__ >= 130
in vec3 nor;
out vec4 FragColor;
#else
varying vec3 nor;
#define FragColor gl_FragColor
#endif


void main() {
    vec3 fpos = gl_FragCoord.xyz / vec3(screen, 1.);
    vec4 xpos = 2.0 * vec4(fpos, 1.0) - 1.0;
    xpos = invproj * xpos;

    xpos.xyz /= xpos.w;
    vec3 viewSampleDir = reflect(xpos.xyz, nor);
    // Convert sampleDir in world space (where tex was generated)
    vec4 sampleDir = TransposeViewMatrix * vec4(viewSampleDir, 0.);
    vec4 detail0 = texture(tex, sampleDir.xyz);

    FragColor = vec4(detail0.xyz, 1.);
}
