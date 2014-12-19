uniform sampler2D ntex;
uniform sampler2D dtex;

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

void main() {
    vec2 uv = gl_FragCoord.xy / screen;
    float z = texture(dtex, uv).x;
    vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);

    vec3 norm = normalize(DecodeNormal(2. * texture(ntex, uv).xy - 1.));
    float roughness = texture(ntex, uv).z;
    vec3 eyedir = -normalize(xpos.xyz);

    vec3 L = normalize((transpose(InverseViewMatrix) * vec4(sun_direction, 0.)).xyz);
    float NdotL = clamp(dot(norm, L), 0., 1.);

    float angle = 3.14 * sun_angle / 180.;
    vec3 R = reflect(-eyedir, norm);
    vec3 Lightdir = getMostRepresentativePoint(L, R, angle);

    vec3 Specular = SpecularBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);
    vec3 Diffuse = DiffuseBRDF(norm, eyedir, Lightdir, vec3(1.), roughness);

    Diff = vec4(NdotL * Diffuse * sun_col, 1.);
    Spec = vec4(NdotL * Specular * sun_col, 1.);

/*	if (hasclouds == 1)
	{
		vec2 cloudcoord = (xpos.xz * 0.00833333) + wind;
		float cloud = texture(cloudtex, cloudcoord).x;
		//float cloud = step(0.5, cloudcoord.x) * step(0.5, cloudcoord.y);

		outcol *= cloud;
	}*/
}
