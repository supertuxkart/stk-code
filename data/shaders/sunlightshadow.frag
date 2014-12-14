uniform sampler2D ntex;
uniform sampler2D dtex;
uniform sampler2DArray shadowtex;

uniform float split0;
uniform float split1;
uniform float split2;
uniform float splitmax;

uniform vec3 direction;
uniform vec3 col;
uniform float sunangle = .54;

in vec2 uv;
out vec4 Diff;
out vec4 Spec;

vec3 DecodeNormal(vec2 n);
vec3 SpecularBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness);
vec3 DiffuseBRDF(vec3 normal, vec3 eyedir, vec3 lightdir, vec3 color, float roughness);
vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

vec3 getMostRepresentativePoint(vec3 direction, vec3 R, float angularRadius)
{
    vec3 D = direction;
    float d = cos(angularRadius);
    float r = sin(angularRadius);
    float DdotR = dot(D, R);
    vec3 S = R - DdotR * D;
    return (DdotR < d) ? normalize(d * D + normalize (S) * r) : R;
}

float getShadowFactor(vec3 pos, int index)
{
    vec4 shadowcoord = (ShadowViewProjMatrixes[index] * InverseViewMatrix * vec4(pos, 1.0));
    shadowcoord.xy /= shadowcoord.w;
    vec2 shadowtexcoord = shadowcoord.xy * 0.5 + 0.5;

    float z = texture(shadowtex, vec3(shadowtexcoord, float(index))).x;
    float d = shadowcoord.z;
    return min(pow(exp(-32. * d) * z, 8.), 1.);
}

void main() {
    vec2 uv = gl_FragCoord.xy / screen;
    float z = texture(dtex, uv).x;
    vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);

    vec3 norm = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));
    float roughness =texture(ntex, uv).z;
    vec3 eyedir = -normalize(xpos.xyz);

    // Normalized on the cpu
    vec3 L = direction;
    float NdotL = clamp(dot(norm, L), 0., 1.);

    float angle = 3.14 * sunangle / 180.;
    vec3 R = reflect(-eyedir, norm);
    vec3 Lightdir = getMostRepresentativePoint(direction, R, angle);

    vec3 Specular = SpecularBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);
    vec3 Diffuse = DiffuseBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);

    // Shadows
    float factor;
    if (xpos.z < split0)
        factor = getShadowFactor(xpos.xyz, 0);
    else if (xpos.z < split1)
        factor = getShadowFactor(xpos.xyz, 1);
    else if (xpos.z < split2)
        factor = getShadowFactor(xpos.xyz, 2);
    else if (xpos.z < splitmax)
        factor = getShadowFactor(xpos.xyz, 3);
    else
        factor = 1.;

    Diff = vec4(factor * NdotL * Diffuse * col, 1.);
    Spec = vec4(factor * NdotL * Specular * col, 1.);
}
