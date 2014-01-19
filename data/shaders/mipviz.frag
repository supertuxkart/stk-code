#version 130

uniform sampler2D tex;
uniform vec2 texsize;
uniform int notex;

float miplevel(in vec2 texture_coordinate)
{
    // The OpenGL Graphics System: A Specification 4.2
    //  - chapter 3.9.11, equation 3.21

    vec2  dx_vtc        = dFdx(texture_coordinate);
    vec2  dy_vtc        = dFdy(texture_coordinate);
    float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));

    return 0.5 * log2(delta_max_sqr); // == log2(sqrt(delta_max_sqr));
}

void main() {

	if (notex != 0) {
		gl_FragColor = gl_Color;
		return;
	}

	// Buggy Intel windows driver workaround
	vec4 levels[6] = vec4[](
		vec4(0.0, 0.0, 1.0, 0.8),
		vec4(0.0, 0.5, 1.0, 0.4),
		vec4(1.0, 1.0, 1.0, 0.0),
		vec4(1.0, 0.7, 0.0, 0.2),
		vec4(1.0, 0.3, 0.0, 0.6),
		vec4(1.0, 0.0, 0.0, 0.8)
		);

	float mip = miplevel(texsize * gl_TexCoord[0].xy) + 2.0;
	mip = clamp(mip, 0.0, 5.0);

	int lowmip = int(mip);
	int highmip = lowmip + 1;
	if (highmip > 5)
		highmip = 5;

	float mixer = fract(mip);

	vec4 mixcol = mix(levels[lowmip], levels[highmip], mixer);
	vec4 tcol = texture(tex, gl_TexCoord[0].xy);

	vec3 col = mix(tcol.xyz, mixcol.xyz, mixcol.a);

	gl_FragColor = vec4(col, tcol.a);
}
